from pprint import pprint
import math
import bisect
import matplotlib.pyplot as plt
from matplotlib.patches import RegularPolygon, Polygon
import numpy as np

clist = ['pink','purple','blue','cyan','green','olive','red','orange','brown','black','gray','grey','yellow',
         'silver','tan','navy','lightpink','peru']

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

#-------------------------------------------------- START reference Henso algorithm -----------------------------------------------------------
Ymax = 852*7.5*0.5;
tang60 = math.sqrt(3);
Xmax = (Ymax*2)/math.sqrt(3);
deltax = Xmax*(2./852);

class GLine:
    __slots__ = ('x_start', 'y_start', 'x_end', 'y_end', 'color', 'val')
    
    def __init__(self, color, val):
        self.color = color
        self.val = val 

        assert self.color != 'D'

        if self.color == 'Y':
            self.y_start = (self.val-426) * 7.5
            self.y_end = (self.val-426) * 7.5
            self.x_start = -Xmax
            self.x_end = +Xmax
        elif self.color == 'R':
            self.y_start = -Ymax
            self.y_end = Ymax
            self.x_start =  Ymax/tang60 - deltax*self.val
            self.x_end =   Xmax + Ymax/tang60 - deltax*self.val
        elif self.color == 'B':
            self.y_start = Ymax
            self.y_end = -Ymax
            self.x_start = -Xmax - Ymax/tang60 + deltax*self.val;
            self.x_end = -Ymax/tang60 + deltax*self.val;

    def __repr__(self):
        return f"GLine({self.color}{self.val}, start=({self.x_start:.2f}, {self.y_start:.2f}), end=({self.x_end:.2f}, {self.y_end:.2f}))"

color_bit = {
    'Y': 0,
    'R': 1,
    'B': 2
}

class GIntersection:
    __slots__ = ('x', 'y', 'flag', 'clustered')

    def __init__(self, gline1, gline2):
        assert gline1.color != gline2.color
        # see https://en.wikipedia.org/wiki/Intersection_(geometry)#Two_lines
        denominator = ((gline1.x_start - gline1.x_end) * (gline2.y_start - gline2.y_end) -
                       (gline1.y_start - gline1.y_end) * (gline2.x_start - gline2.x_end))
                      
        self.x = ((gline1.x_start * gline1.y_end - gline1.y_start * gline1.x_end) * (gline2.x_start - gline2.x_end) -
                  (gline1.x_start - gline1.x_end) * (gline2.x_start * gline2.y_end - gline2.y_start * gline2.x_end))
        self.x = self.x / denominator

        self.y = ((gline1.x_start * gline1.y_end - gline1.y_start * gline1.x_end) * (gline2.y_start - gline2.y_end) -
                  (gline1.y_start - gline1.y_end) * (gline2.x_start * gline2.y_end - gline2.y_start * gline2.x_end))
        self.y = self.y / denominator
        
        
        self.flag = 1<<color_bit[gline1.color]
        self.flag  |= 1<<color_bit[gline2.color]
        
        self.clustered = False
        
    def distance(self, other):
        return math.sqrt((self.x - other.x)**2 + (self.y - other.y)**2)
        
    def __repr__(self):
        return f"GIntersection(x={self.x:.4f}, y={self.y:.4f}, flag={self.flag:03b}"


class GCentroid:
    __slots__ = ('x', 'y', 'cumx', 'cumy', 'weight', 'flag')
    
    def __init__(self, points):
        self.flag = 0
        self.cumx = 0
        self.cumy = 0
        self.x = 0
        self.y = 0
        self.weight = 0
        for point in points:
            self.cumx += point.x
            self.cumy += point.y
            self.flag  |= point.flag
        self.weight += len(points)
        self.x = self.cumx/self.weight
        self.y = self.cumy/self.weight
        
        
    def addPoint(self, point):
        self.cumx += point.x
        self.cumy += point.y
        self.flag  |= point.flag
        self.weight += 1
        
        self.x = self.cumx/self.weight
        self.y = self.cumy/self.weight
        
    def __repr__(self):
        return f"GCentroid(x={self.x:.4f}, y={self.y:.4f}, weight={self.weight}, flag={self.flag:03b})"
        
def findHitsRef(event, cluster_threshold, n, *, plot=False):
    if plot: plt.figure()
    hits = []
    
    # get line coordinnates for each row, col pair
    glines = []
    for pair in event:
        lcolor, lval = line_address[pair]
        if lcolor == 'D':
            print(f"Warning - For {pair}, this is a dummy cell, skipped")
            continue
        glines.append(GLine(lcolor, lval))
    ylines = [gline for gline in glines if gline.color == 'Y']
    rlines = [gline for gline in glines if gline.color == 'R']
    blines = [gline for gline in glines if gline.color == 'B']
    #pprint(glines)
    
    # get intersection points
    intersections = []
    for yline in ylines:
        if plot: plt.plot([yline.x_start, yline.x_end], [yline.y_start, yline.y_end],'--', markersize=0, color='yellow', linewidth=1.2)
        for rline in rlines:
            if plot: plt.plot([rline.x_start, rline.x_end], [rline.y_start, rline.y_end],'--', markersize=0, color='red', linewidth=1.2)
            intersections.append(GIntersection(yline, rline))    
        for bline in blines:
            if plot: plt.plot([bline.x_start, bline.x_end], [bline.y_start, bline.y_end],'--', markersize=0, color='blue', linewidth=1.2)
            intersections.append(GIntersection(yline, bline))
    for rline in rlines:
        if plot: plt.plot([rline.x_start, rline.x_end], [rline.y_start, rline.y_end],'--', markersize=0, color='red', linewidth=1.2)
        for bline in blines:
            if plot: plt.plot([bline.x_start, bline.x_end], [bline.y_start, bline.y_end],'--', markersize=0, color='blue', linewidth=1.2)
            intersections.append(GIntersection(rline, bline))
    #pprint(intersections)
    
    
    if plot: plt.scatter([ inter.x for inter in intersections],  [inter.y for inter in intersections], 20, color='black', marker='o')

    # clusterize
    # algorithm: take an intersection, put all other intersections that
    # are not yet clustered and closer than a given distance `cluster_threshold`
    # in the same cluster. Continue until all intersections are clustered
    clusters = []
    for intersection in intersections:
        if intersection.clustered:
            continue
        cluster = [intersection]
        intersection.clustered = True
        for intersection_ in intersections:
            if intersection_.clustered:
                continue
            if intersection.distance(intersection_) < cluster_threshold:
                intersection_.clustered = True
                cluster.append(intersection_)
        clusters.append(cluster)
    #pprint(clusters)
    
    # compute cluster centroïd, filter for full flag
    hits = []
    for cluster in clusters:
        centroid = GCentroid(cluster)
        if centroid.flag == 0b111:
            hits.append(centroid)
            if plot: plt.scatter(centroid.x, centroid.y, facecolors='cyan', s=100)
            
    #pprint(hits)
    if plot: 
        plt.ylim(-4000,4000)
        plt.xlim(-4000,5000)
        plt.xlabel('X-axis [$\mu$m]')
        plt.ylabel('Y-axis [$\mu$m]')
        plt.title(f"PICMIC$0$ Intersections, Centroids and Clusters, event ${n}$")
        #plt.show()
    return hits
   
#-------------------------------------------------- END reference Henso algorithm -----------------------------------------------------------


def getLines(event):
    """
    The line number are offseted to have an easier invariant.
    With this numbering, we have -1 <= y-b-r <= +1 at every three color intersection
    And we have:
          0  <= b <= 851
          0  <= y <= 851
        -427 <= r <= 424
        
    """
    # get line for each row, col pair
    blines, ylines, rlines = [], [], []
    for pair in event:
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
            lval -= 427
            bisect.insort(rlines, lval)
    return blines, ylines, rlines
       
def transform(x, y):
    """ transform coordinate with a 30° rotation on the y-axis"""
    return x-y/2, y*math.sqrt(3)/2
    #return x, y
    
def findKintersections(all_lines):
    """ Very easy to find triple intersection with good line coordinates"""
    blines, ylines, rlines = all_lines
    kintersections = []
    for y in ylines:
        for b in blines:
            for r in rlines:
                if -2 <= y-b-r <= +2: kintersections.append((b, y, r))
    #pprint(kintersections)
    return kintersections
    
def findClusters(all_lines, fatline_cut, fatintersect_cut):
    
    def cluster1D(lines):
        lines_ = []
        last = None
        fat = None
        for l in lines:
            if last is None:
                last = l
                fat = l
            else:
                if l - last < fatline_cut:
                    last = l
                else:
                    lines_.append((fat, last))
                    last = l
                    fat = l
        if last is not None:
            lines_.append((fat, last))
        return lines_
        
    blines, ylines, rlines = all_lines
    
    fat_blines = cluster1D(blines)
    fat_ylines = cluster1D(ylines)
    fat_rlines = cluster1D(rlines)
    
    # print(fat_ylines)
    # print(fat_blines)
    # print(fat_rlines)
    
    def is_fat_intersect(min0, max0, min1, max1):
       return max0-min0 > fatintersect_cut/10 and max1-min1 > fatintersect_cut/10 and max0-min0 + max1-min1 > fatintersect_cut
       
    clusters = []
    for y0, y1 in fat_ylines:
        for b0, b1 in fat_blines:
            for r0, r1 in fat_rlines:
                # print(r0,r1)
                # print(y0-b0,y1-b1,y1-b0,y0-b1)
                # print()
                if r0 <= y0-b1 <= r1 or  r0 <= y1-b0 <= r1 or y0-b1 <= r0 <= y1-b0 or  y0-b1 <= r1 <= y1-b0: # if the three fat lines intersect
                    edges = []
                    p0, p1 = (max(b0, y0-r1), y0), (min(b1,y0-r0), y0) # on the  y0 line
                    if p0[0] <= p1[0]: edges.append((p0, p1))
                    
                    p0, p1 = (max(b0, y1-r1), y1), (min(b1,y1-r0), y1) # on the y1 line
                    if p0[0] <= p1[0]: edges.append((p0, p1))
                    
                    p0, p1 = (b0, max(y0,r0+b0)), (b0, min(y1, r1+b0)) # on the b0 line
                    if p0[1] <= p1[1]: edges.append((p0, p1))
                    
                    p0, p1 = (b1, max(y0,r0+b1)), (b1, min(y1, r1+b1)) # on the b1 line
                    if p0[1] <= p1[1]: edges.append((p0, p1))
                    
                    p0, p1 = (y0-r0, y0) if y0 >= b0+r0 else (b0, r0+b0), (y1-r0, y1) if y1 <= b1+r0 else (b1, r0+b1) # on the r0 line
                    if p0[0] <= p1[0] and p0[1] <= p1[1]: edges.append((p0, p1))
                    
                    p0, p1 = (y0-r1, y0) if y0 >= b0+r1 else (b0, r1+b0), (y1-r1, y1) if y1 <= b1+r1 else (b1, r1+b1) # on the r1 line
                    if p0[0] <= p1[0] and p0[1] <= p1[1]: edges.append((p0, p1))
                    clusters.append((edges, 0b111))

            if is_fat_intersect(y0,y1,b0,b1): # keep really fat intersection even if only two lines intersect
                edges = []
                p0, p1 = (b0, y0), (b1, y0) # on the  y0 line
                if p0[0] <= p1[0]: edges.append((p0, p1))
                
                p0, p1 = (b0, y1), (b1, y1) # on the y1 line
                if p0[0] <= p1[0]: edges.append((p0, p1))
                
                p0, p1 = (b0, y0), (b0, y1) # on the b0 line
                if p0[1] <= p1[1]: edges.append((p0, p1))
                
                p0, p1 = (b1, y0), (b1,y1) # on the b1 line
                if   p0[1] <= p1[1]: edges.append((p0, p1))
                clusters.append((edges, 0b011))
                    
                
        for r0, r1 in fat_rlines:
            if  is_fat_intersect(y0,y1,r0,r1): # keep really fat intersection even if only two lines intersect
                edges = []
                p0, p1 = (y0-r1, y0), (y0-r0, y0) # on the  y0 line
                if p0[0] <= p1[0]: edges.append((p0, p1))
                
                p0, p1 = (y1-r1, y1), (y1-r0, y1) # on the y1 line
                if p0[0] <= p1[0]: edges.append((p0, p1))

                p0, p1 = (y0-r0, y0) , (y1-r0, y1)  # on the r0 line
                if p0[0] <= p1[0] and p0[1] <= p1[1]: edges.append((p0, p1))
                
                p0, p1 = (y0-r1, y0) , (y1-r1, y1) # on the r1 line
                if p0[0] <= p1[0] and p0[1] <= p1[1]: edges.append((p0, p1))
                clusters.append((edges, 0b101))
                    
                    
    for b0, b1 in fat_blines:
        for r0, r1 in fat_rlines:
            if is_fat_intersect(b0,b1,r0,r1): # keep really fat intersection even if only two lines intersect
                edges = []
                p0, p1 = (b0, r0+b0), (b0,r1+b0) # on the b0 line
                if p0[1] <= p1[1]: edges.append((p0, p1))
                
                p0, p1 = (b1, r0+b1), (b1, r1+b1) # on the b1 line
                if   p0[1] <= p1[1]: edges.append((p0, p1))
                
                p0, p1 =  (b0, r0+b0),  (b1, r0+b1) # on the r0 line
                if p0[0] <= p1[0] and p0[1] <= p1[1]: edges.append((p0, p1))
                
                p0, p1 = (b0, r1+b0), (b1, r1+b1) # on the r1 line
                if p0[0] <= p1[0] and p0[1] <= p1[1]: edges.append((p0, p1))
                clusters.append((edges, 0b110))
    #pprint(clusters)
    
    return clusters
    
    



def plotEvent(n,*, all_lines=[], kintersections=[], clusters=[]):

    sensor_edges =   [(-1, -1), (427, -1), (852, 424), (852, 852), (427, 852), (-1, 424)]
    sensor_edges = [transform(x, y) for x, y in sensor_edges]
    ax = plt.gca()
    ax.add_patch(Polygon(sensor_edges, edgecolor='k', fill=False))
    
    def plot_line(p0, p1, *args, **kwargs):
            x0, y0 = transform(*p0)
            x1, y1 = transform(*p1)
            ax.plot([x0,x1], [y0,y1], *args, **kwargs)
            
    if all_lines:
        blines, ylines, rlines = all_lines
        for bline in blines:
            if bline <= 426:  plot_line((bline, 0), (bline, 424+bline), '-', color='blue', alpha=0.2)
            else: plot_line((bline, bline-427), (bline, 851), '-', color='blue', alpha=0.2)
        for yline in ylines: 
            if yline <= 424:  plot_line((0, yline), (427+yline, yline), '-', color='yellow', alpha=0.2)
            else:  plot_line((yline-424, yline), (851, yline), '-', color='yellow', alpha=0.2)
        for rline in rlines: 
            if rline <= 0: plot_line((-rline, 0), (851, 851+rline), '-', color='red', alpha=0.2)
            else:  plot_line((0, rline), (851-rline, 851), '-', color='red', alpha=0.2)

    if kintersections:
        B, Y = [], []
        for kintersect in kintersections:
            # If we use the blue and yellow lines as a basis for our coordinate system, then not all kintersections
            # are on integer coordinates. In fact it is only the case if yellow-blue = red. Do a diagram if this is not clear
            b, y, r = kintersect
            if y-b > r:
                b, y = b+1/3, y-1/3
            elif y-b < r:
                b, y = b-1/3, y+1/3
            #ax.add_patch(RegularPolygon(transform(b, y), facecolor=clist[cluster%len(clist)], numVertices=3, radius=1, orientation=-math.pi/6))
            B.append(b-y/2)
            Y.append(y*math.sqrt(3)/2)
        ax.scatter(B, Y, marker='o', color='k', alpha=0.2)
            

    if clusters:
        for idx, (edge_list, cluster_type) in enumerate(clusters):
            X,Y = [],[]
            for p0, p1 in edge_list:
                plot_line(p0, p1, '-', color='magenta' if cluster_type==0b111 else 'cyan', zorder=50 if cluster_type==0b111 else 10)
                X.append(p0[0]-p0[1]/2) , X.append(p1[0]-p1[1]/2)
                Y.append(p0[1]*math.sqrt(3)/2) , Y.append(p1[1]*math.sqrt(3)/2)

    ax.set_aspect('equal')
    ax.set_title(f"PICMIC$0$ event ${n}$")
    
    
    
def main():

    file = 'more_events.txt'
    result_dir = file.removesuffix('.txt')
    with open(file, 'r') as fevents:
        found_hit = 0
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
                
            if not (15 <= n+1 <= 50): continue
            #if n+1 != 78: continue
            #if n+1 < 471: continue
            
            print(f"--------------------------- {1+n} ----------------------------------")
            #findHitsRef(event, 100, n+1, plot=True), plt.figure()
            
            lines = getLines(event)
            kintersections = findKintersections(lines)
            clusters = findClusters(lines, fatline_cut=20, fatintersect_cut=45)
            
            #print(clusters)
            plotEvent(n+1, all_lines=lines, clusters=clusters,kintersections=kintersections)
            #plt.savefig(f"{result_dir}/{n+1}.png")
            #plt.clf()
            plt.show()
            
            # if not clusters:
                #
                # #plotEvent(n+1, all_lines=lines, clusters=clusters)#,kintersections=kintersections,)
                # #plt.show()
            # else:
                # found_hit += 1
        # print(f"found hit in {found_hit}/{n} events")
            
if __name__ == "__main__":
    #import cProfile
    #cProfile.run('main()')
    main()
