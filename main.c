#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include "xypicmic.h"

int main(int argc, char *argv[]) {
    //Sanity Checks
    if (argc < 3) {
        printf("Usage: %s <number of elements> <list of row and column pairs>\n", argv[0]);
        return 1;
    }

    int numElements = atoi(argv[1]);
    if (numElements < 1 || argc != 2 + numElements * 2) {
        printf("Invalid number of arguments. Please provide the correct number of row and column pairs.\n");
        return 1;
    }
      
    // ----------------------------------------------------------------  
    // fill array of lines per Event and count lines by color  
    // ----------------------------------------------------------------  
    LineCoordinates *lineInEvent = (LineCoordinates *)malloc(numElements * sizeof(LineCoordinates));
    
    int y_size=0; int r_size=0; int b_size=0; 
    fillLines(argv, lineInEvent, numElements, &y_size, &r_size, &b_size);

    // -----------------------------------------------------------------
    // Arrays of struct allocation (according ot its' color)
    // -----------------------------------------------------------------
    LineCoordinates *ylines = (LineCoordinates *)malloc(y_size * sizeof(LineCoordinates));
    LineCoordinates *rlines = (LineCoordinates *)malloc(r_size * sizeof(LineCoordinates));
    LineCoordinates *blines = (LineCoordinates *)malloc(b_size * sizeof(LineCoordinates));
    
    splitLineColor(lineInEvent,numElements,ylines,rlines,blines);

    //  prinout all lines in event :
    printf("------------------------->>>>  Lines in Event:  <<<<<<<<<<<<<<<<<-----------------\n");
    printf("track;pt0;pt1\n");
    for (int idx=0 ; idx< numElements;  idx++){
        printf("%c%d;(%.02f, %0.2f); (%0.2f, %0.2f)\n",lineInEvent[idx].type,lineInEvent[idx].val , lineInEvent[idx].x_start, lineInEvent[idx].y_start, lineInEvent[idx].x_end, lineInEvent[idx].y_end);
    }

    // -----------------------------------------------------------------
    // compute intersections, centroids and keep these in an array
    // -----------------------------------------------------------------
    int interCount = 0;
    int combinations = y_size*r_size + y_size*b_size + b_size*r_size;
    IntersectionPoint *intersections;
    if (combinations>0){
        intersections = (IntersectionPoint *)malloc(combinations * sizeof(IntersectionPoint));
        xLines(intersections,combinations,ylines,y_size,rlines,r_size,blines,b_size,&interCount);
    }

    printf("------------------------->>>>  Intersections in Event:  <<<<<<<<<<<<<<<<<-----------------\n");
    for (int idx=0 ; idx< interCount;  idx++){
        printf("Flag=%d -- Intersects=%d -- ,x0=%.02f, y0=%0.2f\n",intersections[idx].flag, intersections[idx].intersects, intersections[idx].x, intersections[idx].y);
    }

    IntersectionPoint *centroids;//[interCount];
    //if (interCount>0){
        centroids = (IntersectionPoint *)malloc(interCount * sizeof(IntersectionPoint));
        fillCentroids(intersections,interCount, centroids, interCount );
   // }

    printf("------------------------->>>>  centroids in Event:  <<<<<<<<<<<<<<<<<-----------------\n");
    for (int idx=0 ; idx< interCount;  idx++){
        if ( centroids[idx].flag>0)
        printf(" << ClusNum=%d, Flag=%d -- 3 Lines Different colors=%d -- ,x0=%.02f, y0=%0.2f>>\n",centroids[idx].num,centroids[idx].flag, centroids[idx].intersects, centroids[idx].x, centroids[idx].y);
    }

    printf("============================================================\n");
    //IntersectionPoint *centroidsTest;//[interCount];
    //centroidsTest = (IntersectionPoint *)malloc(interCount * sizeof(IntersectionPoint));
    //assign_clusters(intersections,interCount,centroidsTest,interCount);
    //printIntersectionPoint0(centroids); //<== default printall marche pas
    //printIntersectionPoint(centroids,interCount);

    free(ylines);
    free(rlines);
    free(blines);
    free(intersections);
    free(lineInEvent);
    free(centroids);

    return 0; 
}
