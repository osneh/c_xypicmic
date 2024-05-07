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



void print_event_clusters(const struct event_clusters* clusters);
void fill_event_clusters(struct event_clusters* clusters, struct event_fat_lines* fat_lines);
void print_event_fat_lines(const struct event_fat_lines* fat_lines);
void fill_event_fat_lines(struct event_fat_lines* fat_lines, struct event_lines* lines);
void print_event_lines(const struct event_lines* lines);
void fill_event_lines(struct event_lines* lines, const int* event, size_t event_size);



#endif /* H_XYPICMIC_060524 */
