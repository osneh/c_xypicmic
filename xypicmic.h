#ifndef H_XYPICMIC_060524
#define H_XYPICMIC_060524


#include <stdbool.h>



#define COLS 54
#define ROWS 128
#define MAX_EVENT_SIZE (256*2)
#define MAX_FAT_LINES 10
#define MAX_EVENT_CLUSTERS 20
/* clustering parameters */
#define FATLINE_CUT 40
#define FATINTERSECT_CUT 45


/** @brief Structure holding the PICMIC lines activated in an event */
struct event_lines {
    int ylines[256]; /**< 0° angle 'yellow' lines */
    size_t ysize;    /**< actual length of `ylines` array */
    int blines[256]; /**< 60° angle 'blue' lines */
    size_t bsize;    /**< actual length of `blines` array */
    int rlines[256]; /**< 30° angle 'red' lines */
    size_t rsize;    /**< actual length of `rlines` array */
};

/** @brief Print an event_lines structure to stdout
 *
 *  @param lines
 *      Pointer to an event_lines struct to print to stdout
 *
 */
void print_event_lines(const struct event_lines* lines);


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
void fill_event_lines(struct event_lines* lines, const int* event, size_t event_size);

/** @brief Structure representing a PICMIC fat line
 *
 *  A PICMIC fat line is a grouping of more or less contiguous PICMIC line in the same orientation
 *
 */
struct fat_line {
    int vmin;        /**< first line number in the fat line */
    int vmax;        /**< last line number in the fat line  */
    int width;       /**< width of the fat line             */
    double density;  /**< density of the fat line           */
    bool free;       /**< marks the fat line as already used or not in an intersection */
};

/** @brief Structure representing the PICMIC fat line of an event */
struct event_fat_lines {
    struct fat_line yfat[MAX_FAT_LINES]; /**< 0° angle 'yellow' fat lines      */
    size_t yfat_size;                    /** actual length of the `yfat` array */
    struct fat_line bfat[MAX_FAT_LINES]; /**< 60° angle 'blue' fat lines       */
    size_t bfat_size;                    /** actual length of the `yfat` array */
    struct fat_line rfat[MAX_FAT_LINES]; /**< 30° angle 'red' fat lines        */
    size_t rfat_size;                    /** actual length of the `yfat` array */
};

/** @brief Print an event_fat_lines structure to stdout
 *
 *  @param fat_lines
 *      Pointer to an event_fat_lines struct to print to stdout
 *
 */
void print_event_fat_lines(const struct event_fat_lines* fat_lines);


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
void fill_event_fat_lines(struct event_fat_lines* fat_lines, struct event_lines* lines);

/** @brief Structure representing a PICMIC cluster
 *
 *  A PICMIC event is  list of edges, each described by the x,y coordinates of its endpoints.
 *  Here, we store the x0,y0,x1,y1 values in an array.
 *  Since the cluster is a bounding-box around a fat line intersection, it has necessarly at
 *  most 6 edges, possibly less depending on the details of the intersection
 *
 */
struct cluster {
    int edge[6][4];     /**< array of cluster edges */
    size_t edge_size;   /**< actual length of the edge array */
    char type;          /**< 'colors' involved in the intersection */
};

/** @brief Structure holding all the found cluster in an event */
struct event_clusters {
    struct cluster clusters[MAX_EVENT_CLUSTERS]; /**< array of clusters */
    size_t cluster_size;                         /**< actual length of cluster array */
};

/** @brief Print an event_clusters structure to stdout
 *
 *  @param clusters
 *      Pointer to an event_clusters struct to print to stdout
 *
 */
 
void print_event_clusters(const struct event_clusters* clusters);
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
void fill_event_clusters(struct event_clusters* clusters, struct event_fat_lines* fat_lines);




#endif /* H_XYPICMIC_060524 */
