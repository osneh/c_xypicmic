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
    // fill array of lines per Event   
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
    
    splitLineColor(lineInEvent,numElements, ylines,rlines,blines);

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

    IntersectionPoint *centroids;//[interCount];
    if (interCount>0){
        centroids = (IntersectionPoint *)malloc(interCount * sizeof(IntersectionPoint));
        fillCentroids(intersections,interCount, centroids, interCount );
    }

    free(ylines);
    free(rlines);
    free(blines);
    free(intersections);
    free(lineInEvent);
    free(centroids);

    return 0; 
}