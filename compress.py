from pprint import pprint
import math
import bisect
import itertools


# (row, col) to (color, val) correspondance
line_address = {}
with open('picmic_adress_table.tab', 'r') as faddress:
    # tline = text line
    # gline = geometric line
    for tline in faddress: 
        col, row, n, gline = tline.split()
        col = int(col)
        row = int(row)
        gline = (gline[0], int(gline[2:-1]))
        line_address[(row, col)] = gline

color_bit = {
    'Y': 0b00,
    'R': 0b01,
    'B': 0b10
}

reverse_color_bit = {
    0b00: 'Y',
    0b01: 'R',
    0b10: 'B'
}

def batched_it(iterable, n):
    """
    Batch data into iterators of length n. The last batch may be shorter.
    
    if  Python >= 3.12  use itertools.batched instead
    """
    # batched('ABCDEFG', 3) --> ABC DEF G
    if n < 1:
        raise ValueError('n must be at least one')
    it = iter(iterable)
    while True:
        chunk_it = itertools.islice(it, n)
        try:
            first_el = next(chunk_it)
        except StopIteration:
            return
        yield itertools.chain((first_el,), chunk_it)

class LinePacket:
    __slots__ = ('start', 'color', 'states')

    def __init__(self, start, color):
       self.start = start
       self.color = color
       self.states = []
    
    def __repr__(self):
        states = 0
        for s in self.states: states = (states<<1)|s
        return f"LinePacket( start={self.start}, color={self.color}, states={states:0{len(self.states)}b})"
        
    def encode(self):
        words = [0]
        for state_word in batched_it(self.states, 16):
            w = 0
            for i, s in enumerate(state_word):
                w |= s<<(15-i)
            words.append(w)
        words[0] = (len(words)-1)<<12 | (color_bit[self.color]<<10) | self.start
        return words


def getLines(event):
    """
    The line number are offseted to have an easier invariant.
    With this numbering, we have -1 <= y-b-r <= +1 at every three color intersection
    And we have the following sensor map
          
          
                           (427,851)           Y851             (851,851)                  
                                    +---------------------------+                          
                                   / \                         / \                         
                                  /   \                       /   \                        
                                 /     \                     /     \                       
                                /       \                   /       \                      
                               /         \                 /         \                     
                              /           \               /           \ B851               
                        R424 /             \             / R0          \                   
                            /               \           /               \                  
                           /                 \         /                 \                 
                          /                   \       /                   \                
                         /                     \     /                     \               
                        /                       \   /                       \              
                       /       Y424              \ /                         \             
              (0,424) +---------------------------*---------------------------+ (851,424)  
                       \                         / \                         /             
                        \                       /   \                       /              
                         \                     /     \                     /               
                          \                   /       \                   /                
                           \                 /         \B427             /                 
                            \B0             /           \               /R-427             
                             \             /             \             /                   
                              \           /               \           /                    
                               \         /                 \         /                     
                                \       /                   \       /                      
                                 \     /                     \     /                       
                                  \   /                       \   /                        
                                   \ /           Y0            \ /                         
                                    +---------------------------+                          
                                 (0,0)                            (427,0)                  
        
    """
    # get line for each row, col pair
    blines, ylines, rlines = [], [], []
    last_pair = None
    for pair in event:
        if last_pair == pair:
            print(f"Warning - For {pair}, redundant pair, skipped")
            continue
        last_pair = pair
        lcolor, lval = line_address[pair]
        if lcolor == 'D':
            print(f"Warning - For {pair}, this is a dummy cell, skipped")
            continue
        elif lcolor == 'B':
            lval -= 2
            bisect.insort(blines, lval)
        elif lcolor == 'Y':
            lval -= 1
            bisect.insort(ylines, lval)
        elif lcolor == 'R':
            #lval -= 427
            bisect.insort(rlines, lval)
    return blines, ylines, rlines
       

def compress(all_lines):

   
    def pack_line(lines, color):
        line_states = [ n in lines for n in range(852) ]
        line_packets = []
        line_packet = None
        i = 0
        while i < 852:
            if line_packet is None:
                if line_states[i]: line_packet = LinePacket(i, color)
                i += 1
            else:
                end = min(16, 851-i)
                if line_states[i:i+end] != [0]*end:
                    if len(line_packet.states) < 15*16:
                        line_packet.states.extend(line_states[i:i+end])
                        i += end
                    else:
                        line_packets.append(line_packet)
                        line_packet = LinePacket(i, color)
                        i += 1
                else:
                    line_packets.append(line_packet)
                    line_packet = None
                    i += 1
                    
        if line_packet is not None:
            line_packets.append(line_packet)
        return line_packets
        
    blines, ylines, rlines = all_lines
    
    pack_blines = pack_line(blines, 'B')
    pack_ylines = pack_line(ylines, 'Y')
    pack_rlines = pack_line(rlines, 'R')

    
    return pack_blines + pack_ylines + pack_rlines

def decompress(words):
    blines, ylines, rlines = [], [], []
    
    word_countdown = 0
    current_line_number = None
    current_lines = None
    for word in words:
        if word_countdown == 0:
            current_color = reverse_color_bit[(word>>10) & 0b11]
            current_line_number = word & 0x3FF
            word_countdown = (word>>12) & 0xF
            if current_color == 'B':
               current_lines = blines
            elif current_color == 'Y':
                current_lines = ylines
            elif current_color == 'R':
                current_lines = rlines
            current_lines.append(current_line_number)
        else:
            for i in range(16):
                current_line_number += 1
                if word>>(15-i) & 1: 
                    current_lines.append(current_line_number)
            word_countdown -= 1
    return blines, ylines, rlines

    
def main():

    file = 'more_events.txt'
    cum_size, cum_compress_size = 0,0
    with open(file, 'r') as fevents:
        for n, line in enumerate(fevents):
            if line.strip().startswith('#'):
                continue
            
            nums = list(map(int, line.split()))
            if len(nums) != 2*nums[0] + 1:
                print(nums)
                print("Error - event contains a non consistent list of numbers, skipped")
                continue
            event = []
            for i in range(nums[0]):
                event.append((nums[1+2*i], nums[2+2*i]))
            
            cum_size += 2*len(event)
            print(f"--------------------------- {1+n} ----------------------------------")
            all_lines = getLines(event)
            #pprint(all_lines)
            if not(all_lines[0] or all_lines[1]) or not(all_lines[0] or all_lines[2]) or not(all_lines[1] or all_lines[2]): continue 
            compressed_lines = compress(all_lines)
            pprint(compressed_lines)
            compressed_event = []

            for packet in compressed_lines:
                compressed_event.extend(packet.encode())
                for w in packet.encode():
                    print(f"{w:04X}", end=' ')
                print()

            assert decompress(compressed_event) == all_lines
            cum_compress_size += 2*len(compressed_event)
           
            ratio = 100*(1-(len(compressed_event)/len(event)))
            print(f"Saved {ratio:.02f}%")
    print(f"Saved {cum_size-cum_compress_size}/{cum_size} bytes, data rate saving ~{100*(1-(cum_compress_size/cum_size)):.02f}%")
            
if __name__ == "__main__":
    #import cProfile
    #cProfile.run('main()')
    main()
