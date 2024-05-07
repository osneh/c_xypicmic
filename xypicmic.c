
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "xypicmic.h"
#include "color.h"
#include "mlog.h"



#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))



#define COLS 54
#define ROWS 128
#define MAX_EVENT_SIZE (256*2)
#define MAX_FAT_LINES 10
#define MAX_EVENT_CLUSTERS 20
/* clustering parameters */
#define FATLINE_CUT 40
#define FATINTERSECT_CUT 45


/** < PICMIC row,col pair to line look-up-table */
static const char* picmic_adress_table[ROWS][COLS] = {
    #include "picmic_adress_table.dat"
};

/** @brief compare two integers
 *
 *  Comparison function for qsort
 *
 *  @param a
 *      Void pointer to an integer
 *  @param b
 *      Void pointer to an integer
 *
 *  @returns a-b (as integers)
 */
static int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}


/** @brief Print an event_lines structure to stdout
 *
 *  @param lines
 *      Pointer to an event_lines struct to print to stdout
 *
 */
void print_event_lines(const struct event_lines* lines) {
    printf("EventLines(\n  ylines=[");
    for(size_t y_idx=0; y_idx < lines->ysize; ++y_idx) {
        y_idx != 0 && fputs(", ", stdout);
        printf("%d", lines->ylines[y_idx]);
    }
    printf("]\n  blines=[");
    for(size_t b_idx=0; b_idx < lines->bsize; ++b_idx) {
        b_idx != 0 && fputs(", ", stdout);
        printf("%d", lines->blines[b_idx]);
    }
    printf("]\n  rlines=[");
    for(size_t r_idx=0; r_idx < lines->rsize; ++r_idx) {
        r_idx != 0 && fputs(", ", stdout);
        printf("%d", lines->rlines[r_idx]);
    }
    printf("])\n");
}


/** @brief Fills an event_lines struct with data from an event
 *
 *  The function takes in an array of integers that are interpreted 2 by 2 as 
 *  a row,col pair indentifying a unique line from the PICMIC sensor.
 *  The line numbering are offseted to have an easier invariant.
 *  With this numbering, we have -1 <= y-b-r <= +1 at every three color intersection
 *  And we have the following sensor map
 *        
 *        
 *                         (427,851)           Y851             (851,851)                  
 *                                  +---------------------------+                          
 *                                 / \                         / \                         
 *                                /   \                       /   \                        
 *                               /     \                     /     \                       
 *                              /       \                   /       \                      
 *                             /         \                 /         \                     
 *                            /           \               /           \ B851               
 *                      R424 /             \             / R0          \                   
 *                          /               \           /               \                  
 *                         /                 \         /                 \                 
 *                        /                   \       /                   \                
 *                       /                     \     /                     \               
 *                      /                       \   /                       \              
 *                     /       Y424              \ /                         \             
 *            (0,424) +---------------------------*---------------------------+ (851,424)  
 *                     \                         / \                         /             
 *                      \                       /   \                       /              
 *                       \                     /     \                     /               
 *                        \                   /       \                   /                
 *                         \                 /         \B427             /                 
 *                          \B0             /           \               /R-427             
 *                           \             /             \             /                   
 *                            \           /               \           /                    
 *                             \         /                 \         /                     
 *                              \       /                   \       /                      
 *                               \     /                     \     /                       
 *                                \   /                       \   /                        
 *                                 \ /           Y0            \ /                         
 *                                  +---------------------------+                          
 *                               (0,0)                            (427,0)                  
 *
 *  @param lines
 *      pointer to the event_lines struct to fill
 *  @param event
 *      pointer to the array of integer containing the row, col data of the event
 *  @param event_size 
 *      size of the `event` array
 *
 */
void fill_event_lines(struct event_lines* lines, const int* event, size_t event_size)
{
    memset(lines, 0, sizeof *lines);

    for(size_t i = 0; i < event_size; i += 2) {
        int row = event[i];
        int col = event[i+1];
        const char *line_name = picmic_adress_table[row][col];
        int lval = atoi(&line_name[1]);
        switch(line_name[0]) {
            case 'Y':
                lval -= 1;
                lines->ylines[lines->ysize++] = lval;
            break;
            
            case 'B':
                lval -= 2;
                lines->blines[lines->bsize++] = lval;
            break;
            
            case 'R':
                lval -= 427;
                lines->rlines[lines->rsize++] = lval;
            break;
            
            case 'D':
                LOG_WARN("%s: for (%d, %d), this is a dummy cell, skipped\n", __func__, row, col);
                break;
            default:
                assert(0);
        }
    }
    
    qsort(lines->ylines, lines->ysize, sizeof(int), compare);
    qsort(lines->blines, lines->bsize, sizeof(int), compare);
    qsort(lines->rlines, lines->rsize, sizeof(int), compare);
   
}



/** @brief Print an event_fat_lines structure to stdout
 *
 *  @param fat_lines
 *      Pointer to an event_fat_lines struct to print to stdout
 *
 */
void print_event_fat_lines(const struct event_fat_lines* fat_lines) {
    printf("EventFatLines(\n  fat_ylines=[");
    for(size_t y_idx=0; y_idx < fat_lines->yfat_size; ++y_idx) {
        y_idx != 0 && fputs(", ", stdout);
        printf("(vmin=%d, vmax=%d, density=%.2f, free=%s)", fat_lines->yfat[y_idx].vmin, fat_lines->yfat[y_idx].vmax, fat_lines->yfat[y_idx].density, fat_lines->yfat[y_idx].free ? "true" : "false");
    }
    printf("]\n  fat_blines=[");
    for(size_t b_idx=0; b_idx < fat_lines->bfat_size; ++b_idx) {
        b_idx != 0 && fputs(", ", stdout);
        printf("(vmin=%d, vmax=%d, density=%.2f, free=%s)", fat_lines->bfat[b_idx].vmin, fat_lines->bfat[b_idx].vmax, fat_lines->bfat[b_idx].density, fat_lines->bfat[b_idx].free ? "true" : "false");
        
    }
    printf("]\n  fat_rlines=[");
    for(size_t r_idx=0; r_idx < fat_lines->rfat_size; ++r_idx) {
        r_idx != 0 && fputs(", ", stdout);
        printf("(vmin=%d, vmax=%d, density=%.2f, free=%s)", fat_lines->rfat[r_idx].vmin, fat_lines->rfat[r_idx].vmax, fat_lines->rfat[r_idx].density, fat_lines->rfat[r_idx].free ? "true" : "false");
    }
    printf("])\n");
}


/** @brief Fills an event_fat_lines struct with data from an even_lines struct
 *
 *  This is in effect a 1D clustering on each of the PICMIC sensor orientation.
 *  The goal is to reduce the combinatorics explosion of finding every intersection
 *  by regrouping nearby lines together, where "nearby" is a parameter fixed by 
 *  `FATLINE_CUT`
 *
 *  @param fat_lines
 *      pointer to the event_fat_lines struct to fill
 *  @param lines
 *      pointer to the event_lines struct containing the event individual lines
 *
 */
void fill_event_fat_lines(struct event_fat_lines* fat_lines, struct event_lines* lines)
{
    memset(fat_lines, 0, sizeof *fat_lines);
    size_t fill_fat_lines(struct fat_line* fats, int* lines, size_t lines_size) {
        
        size_t nb_fat_line = 0;
        int first = 0xFFFF, last = 0xFFFF;
        int nline = 0;
        for(size_t l = 0; l < lines_size; ++l) {
            if (last == 0xFFFF) {
                last = lines[l];
                first = lines[l];
                nline = 1;
            } else {
                if ((lines[l]-last) < FATLINE_CUT) {
                    last = lines[l];
                    nline += 1;
                } else {
                    if (nb_fat_line >= MAX_FAT_LINES) {
                        LOG_ERROR("%s: more fat lines than expected, event can't be correctly analysed\n", __func__);
                        return nb_fat_line;
                    }
                    fats[nb_fat_line++] = (struct fat_line){
                    .vmin=first,
                    .vmax=last,
                    .width=last-first+1,
                    .density=(double)nline/(last-first+1),
                    .free=true};
                    last = lines[l];
                    first = lines[l];
                    nline = 1;
                }
            }
        }   
        if (last != 0xFFFF) {
            if (nb_fat_line >= MAX_FAT_LINES) {
                LOG_ERROR("%s: more fat lines than expected, event can't be correctly analysed\n", __func__);
                return nb_fat_line;;
            }
            fats[nb_fat_line++] = (struct fat_line){
                .vmin=first,
                .vmax=last,
                .width=last-first+1,
                .density=(double)nline/(last-first+1),
                .free=true};
        }
        return nb_fat_line;
    }
    
    fat_lines->yfat_size = fill_fat_lines(fat_lines->yfat, lines->ylines, lines->ysize);
    fat_lines->bfat_size = fill_fat_lines(fat_lines->bfat, lines->blines, lines->bsize);
    fat_lines->rfat_size = fill_fat_lines(fat_lines->rfat, lines->rlines, lines->rsize);
}




/** @brief Print an event_clusters structure to stdout
 *
 *  @param clusters
 *      Pointer to an event_clusters struct to print to stdout
 *
 */
void print_event_clusters(const struct event_clusters* clusters) {
    printf("EventClusters([\n");
    for(size_t i=0; i < clusters->cluster_size; ++i) {
        i != 0 && fputs(",\n", stdout);
        const struct cluster* ptc = &clusters->clusters[i];
        fputs("([", stdout);
        for(size_t e=0; e < ptc->edge_size; ++e) {
            e != 0 && fputs(", ", stdout);
            printf("((%d, %d), (%d, %d))", ptc->edge[e][0],  ptc->edge[e][1],  ptc->edge[e][2],  ptc->edge[e][3]);
        }
        printf("], type=%d)", ptc->type);
    }
    printf("])\n");
}

/** @brief Fills an event_clusters struct with fat lines intersections
 *
 *  This is were the meat of the clustering happens. Note that since we defined
 *  a cluster as the bounding-box of a fat line intersection, very little number
 *  crunching computation actually occurs, it is mostly a whole lot comparison to
 *  handle the different cases.
 *
 *  @param clusters
 *      pointer to the event_clusters struct to fill
 *  @param fat_lines
 *      pointer to the event_fat_lines struct containing the fat lines we are trying to
 *      find the intersection of.
 *
 */
void fill_event_clusters(struct event_clusters* clusters, struct event_fat_lines* fat_lines)
{
    memset(clusters, 0, sizeof *clusters);
    
    /* full 3 orientation intersection */
    for(size_t y_idx=0; y_idx < fat_lines->yfat_size; ++y_idx) {
        int y0 = fat_lines->yfat[y_idx].vmin;
        int y1 = fat_lines->yfat[y_idx].vmax;
        for(size_t b_idx=0; b_idx < fat_lines->bfat_size; ++b_idx) {
            int b0 = fat_lines->bfat[b_idx].vmin;
            int b1 = fat_lines->bfat[b_idx].vmax;
            for(size_t r_idx=0; r_idx < fat_lines->rfat_size; ++r_idx) {
                int r0 = fat_lines->rfat[r_idx].vmin;
                int r1 = fat_lines->rfat[r_idx].vmax;
                
                /* if the three fat lines intersect */
                if (((r0 <= y0-b1) && (y0-b1 <= r1)) ||
                    ((r0 <= y1-b0) && (y1-b0 <= r1)) ||
                    ((y0-b1 <= r0) && (r0 <= y1-b0)) ||
                    ((y0-b1 <= r1) && (r1 <= y1-b0))) { 
                    
                    int p0[2], p1[2];
                    struct cluster* ptc =  &clusters->clusters[clusters->cluster_size];
                    
                    /*on the  y0 line */
                    p0[0] = MAX(b0, y0-r1); p0[1] = y0;
                    p1[0] = MIN(b1, y0-r0); p1[1] =  y0;
                    if (p0[0] <= p1[0]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the y1 line */
                    p0[0] = MAX(b0, y1-r1); p0[1] = y1;
                    p1[0] = MIN(b1, y1-r0); p1[1] =  y1;
                    if (p0[0] <= p1[0]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the b0 line */
                    p0[0] = b0; p0[1] = MAX(y0,r0+b0);
                    p1[0] = b0; p1[1] = MIN(y1, r1+b0) ;
                    if (p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the b1 line */
                    p0[0] = b1; p0[1] = MAX(y0,r0+b1);
                    p1[0] = b1; p1[1] = MIN(y1, r1+b1);
                    if (p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the r0 line */
                    if ( y0 >= b0+r0) {
                        p0[0] = y0-r0; p0[1] = y0;
                    } else {
                        p0[0] = b0; p0[1] = r0+b0;
                    }
                    if (y1 <= b1+r0) {
                        p1[0] = y1-r0; p1[1] = y1;
                    } else {
                        p1[0] = b1; p1[1] = r0+b1;
                    }
                    if (p0[0] <= p1[0] && p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the r1 line */
                    if ( y0 >= b0+r1) {
                        p0[0] = y0-r1; p0[1] = y0;
                    } else {
                        p0[0] = b0; p0[1] = r1+b0;
                    }
                    if (y1 <= b1+r1) {
                        p1[0] = y1-r1; p1[1] = y1;
                    } else {
                        p1[0] = b1; p1[1] = r1+b1;
                    }
                    if (p0[0] <= p1[0] && p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    
                    // heuristic to keep considering fat line for simple intersection if intersection too asymetrical or not dense enough
                    struct fat_line *rfatp =  &fat_lines->rfat[r_idx], *bfatp = &fat_lines->bfat[b_idx], *yfatp = &fat_lines->yfat[y_idx];
                    int minw = MIN(MIN(bfatp->width, yfatp->width), rfatp->width);
                    rfatp->free = rfatp->width > minw*10  && rfatp->density < 0.4;
                    bfatp->free = bfatp->width > minw*10  && bfatp->density < 0.4;
                    yfatp->free = yfatp->width > minw*10  && yfatp->density < 0.4;
                    
                    ptc->type = 0b111;
                    ++clusters->cluster_size;
                    if (clusters->cluster_size >= MAX_EVENT_CLUSTERS) {
                        LOG_ERROR("%s: more clusters than expected, event can't be correctly analysed\n", __func__);
                        return;
                    }
                    
                }
            }
        }
    }
        

    bool is_fat_intersect(const struct fat_line* f0, const struct fat_line* f1) {
        return f0->free && f1->free && MIN(f0->width, f1->width)*FATLINE_CUT >= MAX(f0->width, f1->width) && f0->width+f1->width > FATINTERSECT_CUT;
    }
    
    
    for(size_t y_idx=0; y_idx < fat_lines->yfat_size; ++y_idx) {
        int y0 = fat_lines->yfat[y_idx].vmin;
        int y1 = fat_lines->yfat[y_idx].vmax;
        
        /* yellow-blue intersection */
        for(size_t b_idx=0; b_idx < fat_lines->bfat_size; ++b_idx) {
            int b0 = fat_lines->bfat[b_idx].vmin;
            int b1 = fat_lines->bfat[b_idx].vmax;
            
            if (is_fat_intersect(&fat_lines->yfat[y_idx], &fat_lines->bfat[b_idx])) {
                int p0[2], p1[2];
                    struct cluster* ptc =  &clusters->clusters[clusters->cluster_size];
                    
                    /*on the  y0 line */
                    p0[0] = b0; p0[1] = y0;
                    p1[0] = b1; p1[1] = y0;
                    if (p0[0] <= p1[0]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the y1 line */
                    p0[0] = b0; p0[1] = y1;
                    p1[0] = b1; p1[1] = y1;
                    if (p0[0] <= p1[0]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the b0 line */
                    p0[0] = b0; p0[1] = y0;
                    p1[0] = b0; p1[1] = y1;
                    if (p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the b1 line */
                    p0[0] = b1; p0[1] = y0;
                    p1[0] = b1; p1[1] = y1;
                    if (p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    ptc->type = 0b011;
                    ++clusters->cluster_size;
                    if (clusters->cluster_size >= MAX_EVENT_CLUSTERS) {
                        LOG_ERROR("%s: more clusters than expected, event can't be correctly analysed\n", __func__);
                        return;
                    }
            }
        }
        
        /* yellow-red intersection */
        for(size_t r_idx=0; r_idx < fat_lines->rfat_size; ++r_idx) {
            int r0 = fat_lines->rfat[r_idx].vmin;
            int r1 = fat_lines->rfat[r_idx].vmax;
            
            if (is_fat_intersect(&fat_lines->yfat[y_idx], &fat_lines->rfat[r_idx])) {
                int p0[2], p1[2];
                    struct cluster* ptc =  &clusters->clusters[clusters->cluster_size];
                    
                    /*on the  y0 line */
                    p0[0] = y0-r1; p0[1] = y0;
                    p1[0] = y0-r0; p1[1] =  y0;
                    if (p0[0] <= p1[0]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the y1 line */
                    p0[0] = y1-r1; p0[1] = y1;
                    p1[0] = y1-r0; p1[1] =  y1;
                    if (p0[0] <= p1[0]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                   /* on the r0 line */
                    p0[0] = y0-r0; p0[1] = y0;
                    p1[0] = y1-r0; p1[1] = y1;
                    if (p0[0] <= p1[0] && p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    /* on the r1 line */
                    p0[0] = y0-r1; p0[1] = y0;
                    p1[0] = y1-r1; p1[1] = y1;
                    if (p0[0] <= p1[0] && p0[1] <= p1[1]) {
                        ptc->edge[ptc->edge_size][0] = p0[0];
                        ptc->edge[ptc->edge_size][1] = p0[1];
                        ptc->edge[ptc->edge_size][2] = p1[0];
                        ptc->edge[ptc->edge_size][3] = p1[1];
                        ++ptc->edge_size;
                    }
                    
                    ptc->type = 0b101;
                    ++clusters->cluster_size;
                    if (clusters->cluster_size >= MAX_EVENT_CLUSTERS) {
                        LOG_ERROR("%s: more clusters than expected, event can't be correctly analysed\n", __func__);
                        return;
                    }
            }
        }
    }
    
    /* blue-red intersection */
    for(size_t b_idx=0; b_idx < fat_lines->bfat_size; ++b_idx) {
        int b0 = fat_lines->bfat[b_idx].vmin;
        int b1 = fat_lines->bfat[b_idx].vmax;   
        for(size_t r_idx=0; r_idx < fat_lines->rfat_size; ++r_idx) {
            int r0 = fat_lines->rfat[r_idx].vmin;
            int r1 = fat_lines->rfat[r_idx].vmax;
            
            if (is_fat_intersect(&fat_lines->bfat[b_idx], &fat_lines->rfat[r_idx])) {
                int p0[2], p1[2];
                struct cluster* ptc =  &clusters->clusters[clusters->cluster_size];
                
                /* on the b0 line */
                p0[0] = b0; p0[1] = r0+b0;
                p1[0] = b0; p1[1] = r1+b0;
                if (p0[1] <= p1[1]) {
                    ptc->edge[ptc->edge_size][0] = p0[0];
                    ptc->edge[ptc->edge_size][1] = p0[1];
                    ptc->edge[ptc->edge_size][2] = p1[0];
                    ptc->edge[ptc->edge_size][3] = p1[1];
                    ++ptc->edge_size;
                }
                
                /* on the b1 line */
                p0[0] = b1; p0[1] = r0+b1;
                p1[0] = b1; p1[1] = r1+b1;
                if (p0[1] <= p1[1]) {
                    ptc->edge[ptc->edge_size][0] = p0[0];
                    ptc->edge[ptc->edge_size][1] = p0[1];
                    ptc->edge[ptc->edge_size][2] = p1[0];
                    ptc->edge[ptc->edge_size][3] = p1[1];
                    ++ptc->edge_size;
                }
                
               /* on the r0 line */
                p0[0] = b0; p0[1] = r0+b0;
                p1[0] = b1; p1[1] = r0+b1;
                if (p0[0] <= p1[0] && p0[1] <= p1[1]) {
                    ptc->edge[ptc->edge_size][0] = p0[0];
                    ptc->edge[ptc->edge_size][1] = p0[1];
                    ptc->edge[ptc->edge_size][2] = p1[0];
                    ptc->edge[ptc->edge_size][3] = p1[1];
                    ++ptc->edge_size;
                }
                
                /* on the r1 line */
                p0[0] = b0; p0[1] = r1+b0;
                p1[0] = b1; p1[1] = r1+b1;
                if (p0[0] <= p1[0] && p0[1] <= p1[1]) {
                    ptc->edge[ptc->edge_size][0] = p0[0];
                    ptc->edge[ptc->edge_size][1] = p0[1];
                    ptc->edge[ptc->edge_size][2] = p1[0];
                    ptc->edge[ptc->edge_size][3] = p1[1];
                    ++ptc->edge_size;
                }
                
                ptc->type = 0b110;
                ++clusters->cluster_size;
                if (clusters->cluster_size >= MAX_EVENT_CLUSTERS) {
                    LOG_ERROR("%s: more clusters than expected, event can't be correctly analysed\n", __func__);
                    return;
                }
            }
        }
    }
}



