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

    if (argc < 3) {
        printf("Usage: %s <number of elements> <list of row and column pairs>\n", argv[0]);
        return 1;
    }

    int numElements = atoi(argv[1]);
    if (numElements < 1 || argc != 2 + numElements * 2) {
        printf("Invalid number of arguments. Please provide the correct number of row and column pairs.\n");
        return 1;
    }

    // get dir path
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return -1;
    }
    replaceBackslashes(cwd); 

    // recover picmic adresses table
    char fileTablePath[PATH_MAX];
    snprintf(fileTablePath, sizeof(fileTablePath), "%s/picmic_adress_table.tab", cwd);

    char arr[ROWS][COLS][MAX_NAME_LENGTH] = {0};
    LineCoordinates lineInEvent[numElements];
    int y_size=0; int r_size=0; int b_size=0; 

    FILE *file = fopen(fileTablePath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Filling array of structure RowCol provided the alias
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char name[MAX_NAME_LENGTH], RYBi[MAX_NAME_LENGTH];
        int col, row;
        sscanf(line, "%d\t%d\t%*d\t%s", &col, &row, name); 
        extractRYBi(name, RYBi); 
        strcpy(arr[row][col], RYBi); 
    }
    fclose(file); 

    // compute lines according color
    for (int i = 0; i < numElements; i++) {
        int inputRow = atoi(argv[2 + i * 2]); 
        int inputCol = atoi(argv[3 + i * 2]); 

        if (inputRow >= 0 && inputRow < ROWS && inputCol >= 0 && inputCol < COLS) {
            char *value = arr[inputRow][inputCol]; 
            char lineType = value[0]; 

            // Traitement en fonction du type de ligne.
            if (lineType == 'D') {
                printf("For Row %d, Column %d: This is a dummy cell.\n", inputRow, inputCol);
            } else {
                int lineValue = atoi(&value[1]); // Conversion de la valeur de la ligne en entier.
                
                // filling integer for a posterior allocation in array of struct
                if (lineType == 'Y') y_size+=1;
                else if (lineType == 'R') r_size+=1;
                else if (lineType == 'B') b_size+=1;
                
                LineCoordinates coords = calculateLineCoordinates(lineType, lineValue); // Calcul des coordonnées de la ligne.
                lineInEvent[i] = coords;
            }
        } else {
            printf("Invalid row or column for element %d. Please enter values within the range.\n", i + 1);
        }
    }

    // -----------------------------------------------------------------
    // Arrays of struct allocation (according ot its' color)
    // -----------------------------------------------------------------
    LineCoordinates *ylines = (LineCoordinates *)malloc(y_size * sizeof(LineCoordinates));
    LineCoordinates *rlines = (LineCoordinates *)malloc(r_size * sizeof(LineCoordinates));
    LineCoordinates *blines = (LineCoordinates *)malloc(b_size * sizeof(LineCoordinates));

    int y_idx =0;int r_idx =0;int b_idx = 0;
    splitLineColor(lineInEvent,numElements, ylines,&y_idx,rlines,&r_idx,blines,&b_idx);

    // -----------------------------------------------------------------
    // compute intersections and keep these in an array
    // -----------------------------------------------------------------
    int interCount = 0;
    int combinations = y_size*r_size + y_size*b_size + b_size*r_size;
    IntersectionPoint *intersections;
    if (combinations>0){
        intersections = (IntersectionPoint *)malloc(combinations * sizeof(IntersectionPoint));
        //IntersectionPoint intersections[combinations];
        xLines(intersections,combinations,ylines,y_size,rlines,r_size,blines,b_size,&interCount);
    }
    //printf("=======================================================\n");
    //printf("counter=%d\n",interCount);
    //for ( int z = 0 ; z<interCount; z++){
    // printf("z=%d,x=%0.2f,y=%0.2f\n", z,intersections[z].x, intersections[z].y);
    //}

    // ----------------------------------------------------
    // ---------------------------------------------
    int clustered[interCount];
	memset(clustered,0,interCount*sizeof(int));

    for (int i = 0; i < interCount; i++) { //printf("HERE12\n");
        if (!clustered[i]) {
            IntersectionPoint cluster[interCount];      // Tableau temporaire pour stocker un cluster de points.
            int clusterSize = 0;                        // Taille du cluster.
            cluster[clusterSize++] = intersections[i];  // Ajout du point initial au cluster.
            clustered[i] = 1;                           // Marquage du point comme regroupé.
            // Recherche d'autres points à inclure dans le cluster.
            for (int j = i + 1; j < interCount; j++) {
                if (!clustered[j]) {
                    double dist = distance(intersections[i].x, intersections[i].y, intersections[j].x, intersections[j].y);
                    if (dist <  THRESHOLD) {
                        cluster[clusterSize++] = intersections[j]; // Ajout du point au cluster.
                        clustered[j] = 1;                           // Marquage du point comme regroupé.
                    }
                }
            }
            IntersectionPoint centroid = calculateCentroid(cluster, clusterSize);
            printf("Centroid --> x=%0.2f ,\t y=%0.2f\n",centroid.x,centroid.y);
        }
    }

    //------------------------------------------------
    printf("=========== My Test ============== \n");
    //int myclustered[interCount];
    //memset(myclustered,0,interCount*sizeof(int));
    IntersectionPoint centroids[interCount];
    fillCentroids(intersections,interCount, centroids, interCount );
    
    for (int n = 0; n<interCount;n++){
        printf("n=%d\t x=%0.2f \t y=%0.2f\n",n,centroids[n].x, centroids[n].y);
    }
    //printf("Centroid --> x=%0.2f ,\t y=%0.2f\n",centroid.x,centroid.y);




    free(ylines);
    free(rlines);
    free(blines);
    free(intersections);

    return 0; 
}