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

    //int threshold = atoi(argv[1]);
    int numElements = atoi(argv[2]);
    int threshold = selThreshold(numElements);
    
    if (numElements < 1 || argc !=3 + numElements * 2) {
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

    //  printout all lines in event :
    char filename[]="xlines.csv";
    FILE *csvFile = fopen(filename, "w");
        if (csvFile == NULL) {
            perror("Error opening CSV file");
            return -1;
        }

    char filename1[]="inter.csv";
    FILE *csvFile1 = fopen(filename1, "w");
    if (csvFile1 == NULL) {
        perror("Error opening CSV file");
        return -1;
    }

    char filename2[]="centroid.csv";
    FILE *csvFile2 = fopen(filename2, "w");
    if (csvFile2 == NULL) {
        perror("Error opening CSV file");
        return -1;
    }
    /*
    char filename3[]="clusters.csv";
    FILE *csvFile3 = fopen(filename3, "w");
    if (csvFile3 == NULL) {
        perror("Error opening CSV file");
        return -1;
    }
    */
    

    printf("------------------------->>>>  Lines in Event:  <<<<<<<<<<<<<<<<<-----------------\n");
    printf("track;pt0;pt1\n");
    fprintf(csvFile, "track;pt0;pt1\n"); 
    for (int idx=0 ; idx< numElements;  idx++){
        fprintf(csvFile,"%c%d;(%.02f, %0.2f); (%0.2f, %0.2f)\n",lineInEvent[idx].type,lineInEvent[idx].val , lineInEvent[idx].x_start, lineInEvent[idx].y_start, lineInEvent[idx].x_end, lineInEvent[idx].y_end);
        printf("%c%d;(%.02f, %0.2f); (%0.2f, %0.2f)\n",lineInEvent[idx].type,lineInEvent[idx].val , lineInEvent[idx].x_start, lineInEvent[idx].y_start, lineInEvent[idx].x_end, lineInEvent[idx].y_end);
    }
    fclose(csvFile);

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
    else {
    	printf("NOT COMBINATIONS \n");
	return 1;
    }

    printf("------------------------->>>>  Intersections:  <<<<<<<<<<<<<<<<<-----------------\n");
    printf("intercoutn=%d\n",interCount);
    fprintf(csvFile1, "x;y\n"); 
    for (int idx=0 ; idx< interCount;  idx++){
        printf("indx=%d, intersects:%d -- ,x0=%.02f, y0=%0.2f\n", idx,intersections[idx].intersects, intersections[idx].x, intersections[idx].y);
        fprintf(csvFile1,"%.04f;%0.4f\n", intersections[idx].x, intersections[idx].y); 
    }
    fclose(csvFile1);

    printf("------------------------->>>>  Centroids :  <<<<<<<<<<<<<<<<<-----------------\n");
    
    fprintf(csvFile2, "numCluster;centroidFlag; centroid3Colors;x;y\n"); 
    if (interCount>0){
    	IntersectionPoint *centroids;//[interCount];
    	centroids = (IntersectionPoint *)malloc(interCount * sizeof(IntersectionPoint));
    	init_array(centroids,interCount);    
    	if (interCount==1) 
	        centroids[0] = intersections[0];
    	else  
    	    fillCentroids(threshold, intersections,interCount, centroids, interCount );
   //}

    	printf("numCluster;centroidFlag; centroid3Colors;x;y\n"); 
    	//fprintf(csvFile2, "numCluster;centroidFlag; centroid3Colors;x;y\n"); 
        //fprintf(csvFile3,"numCluster;x;y\n");
    	for (int idx=0 ; idx< interCount;  idx++){
        	if ( centroids[idx].num>-1 && centroids[idx].flag == 7 ){
        	//if ( centroids[idx].num>-1  ){
        		printf("%d;%d;%d;%.04f;%0.4f\n",centroids[idx].num,centroids[idx].flag, centroids[idx].intersects, centroids[idx].x, centroids[idx].y);
        		fprintf(csvFile2,"%d;%d;%d;%.04f;%0.4f\n",centroids[idx].num,centroids[idx].flag, centroids[idx].intersects, centroids[idx].x, centroids[idx].y);
	 	}
	 }
    	free(centroids);
    }
    fclose(csvFile2);
    //fclose(csvFile3);

    free(ylines);
    free(rlines);
    free(blines);
    free(intersections);
    free(lineInEvent);

    return 0; 
}
