#include "xypicmic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


void replaceBackslashes(char *str) {
    while (*str) {
        if (*str == '\\') *str = '/';
        str++;
    }
}

void extractRYBi(const char *name, char *RYBi) {
    const char *start = strchr(name, '<');
    const char *end = strchr(name, '>');
    if (start != NULL && end != NULL && start < end) {
        int length = end - start - 1;
        if (length < MAX_NAME_LENGTH - 2) {
            RYBi[0] = name[0];
            strncpy(&RYBi[1], start + 1, length);
            RYBi[length + 1] = '\0';
        }
    }
}

double distance(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

LineCoordinates calculateLineCoordinates(char lineType, int value) {
    LineCoordinates coords;
    
    double Ymax = 852*7.5*0.5;
    double tang60 = sqrt(3);
    double Xmax = (Ymax*2)/sqrt(3);
    double deltax = Xmax*(2./852);
    coords.type = lineType;
    coords.val = value;

    if (lineType == 'Y') {
        coords.y_start = coords.y_end = (value-426) * 7.5;
        coords.x_start = -Xmax; 
        coords.x_end = Xmax;
    } else if (lineType == 'R') {
        coords.y_start = -Ymax; coords.y_end = Ymax;
        coords.x_start =  Ymax/tang60 - deltax*value;
        coords.x_end =   Xmax + Ymax/tang60- deltax*value;
    } else if (lineType == 'B') {
        coords.y_start = Ymax; coords.y_end = -Ymax;
        coords.x_start =   -Xmax-Ymax/tang60 +deltax*(value);
        coords.x_end =    -Ymax/tang60 +deltax*(value);
    }

    return coords;
}

IntersectionPoint calculateIntersection(LineCoordinates line1, LineCoordinates line2) {
    IntersectionPoint result = {-99999, -99999, false};
    double denominator = (line1.x_start - line1.x_end) * (line2.y_start - line2.y_end) -
                         (line1.y_start - line1.y_end) * (line2.x_start - line2.x_end);

    if (fabs(denominator) < 1e-6) {
        return result; // Les lignes sont parallèles ou coïncidentes.
    }

    double x = ((line1.x_start * line1.y_end - line1.y_start * line1.x_end) * (line2.x_start - line2.x_end) -
                (line1.x_start - line1.x_end) * (line2.x_start * line2.y_end - line2.y_start * line2.x_end)) / denominator;
    double y = ((line1.x_start * line1.y_end - line1.y_start * line1.x_end) * (line2.y_start - line2.y_end) -
                (line1.y_start - line1.y_end) * (line2.x_start * line2.y_end - line2.y_start * line2.x_end)) / denominator;

    result.x = x;
    result.y = y;
    result.intersects = true;

    return result;
}

IntersectionPoint calculateCentroid(IntersectionPoint *cluster, int size) {
    IntersectionPoint centroid = {0, 0, false};
    
    for (int i = 0; i < size; i++) {
        centroid.x += cluster[i].x;
        centroid.y += cluster[i].y;
    }
    if (size!=0){    
        centroid.x /= size;
        centroid.y /= size;
    }
    else {
        centroid.x = -999999;
        centroid.y = -999999;
    }
    
    return centroid;
}

//int y_idx = 0;int r_idx = 0;int b_idx = 0;
// -----------------------------
//void splitLineColor(unsigned int y, LineCoordinates *y1, unsigned int r, LineCoordinates *r1, unsigned int b, LineCoordinates *b1, LineCoordinates *Items, unsigned int nItems) {
//void splitLineColor(LineCoordinates *Items, unsigned int nItems, LineCoordinates *y1, unsigned int y , LineCoordinates *r1, unsigned int r, LineCoordinates *b1, unsigned int b) {
void splitLineColor(LineCoordinates *Items, int nItems, LineCoordinates *ly, int *y , LineCoordinates *lr, int *r, LineCoordinates *lb, int *b) {
    int unsigned temp_y = 0; unsigned int temp_r=0; unsigned int temp_b=0;
    for ( int j = 0 ; j < nItems; ++j){
        char ltype =  Items[j].type ;
        //printf("--%c%d\n",ltype,Items[j].val);
    
        if ( ltype == 'Y') ly[temp_y++] = Items[j];
        else if ( ltype == 'R' ) lr[temp_r++] = Items[j];
        else if ( ltype == 'B' ) lb[temp_b++] = Items[j];
    }
    *y=temp_y;
    *r=temp_r; 
    *b=temp_b;
}

void xLines(IntersectionPoint *intersecs, int nIntersecs,LineCoordinates *yellow, int y_size, LineCoordinates *red, int r_size, LineCoordinates * blue, int b_size, int * counter){
        int iCount =0;
        if (y_size>0){
            for (int idx = 0 ; idx<y_size; idx++){              // Yellow lines Loop
                for (int jdx = 0 ; jdx<r_size; jdx++){          // Red lines Loop 
                    IntersectionPoint intersection = calculateIntersection(yellow[idx],red[jdx]);
                    intersecs[iCount++] = intersection;
                }

                for (int kdx = 0 ; kdx<b_size; kdx++){          // Blue lines Loop 
                    IntersectionPoint intersection = calculateIntersection(yellow[idx],blue[kdx]);
                    intersecs[iCount++] = intersection;
                }
            }
        }

        if (r_size>0 && b_size > 0 ) {              // Red and Blue lines Loop
            for (int idx = 0; idx<r_size; idx++){
                for (int jdx = 0 ; jdx<b_size; jdx++){
                    IntersectionPoint intersection = calculateIntersection(red[idx],blue[jdx]);
                    intersecs[iCount++] = intersection; 
                }
            }
        }
    *counter=iCount;
}

void fillCentroids(IntersectionPoint *myIntersections, int myDimIntersections,IntersectionPoint * arrayCentroid, int nCentroid ){

    int myclustered[myDimIntersections];
    memset(myclustered,0,myDimIntersections*sizeof(int));
    unsigned int fillCounter = 0;

    for (int i = 0; i < myDimIntersections; i++) { //printf("HERE12\n");
        if (!myclustered[i]) {
            IntersectionPoint cluster[myDimIntersections];      // Tableau temporaire pour stocker un cluster de points.
            int clusterSize = 0;                                // Taille du cluster.
            cluster[clusterSize++] = myIntersections[i];        // Ajout du point initial au cluster.
            myclustered[i] = 1;                                 // Marquage du point comme regroupé.
            // Recherche d'autres points à inclure dans le cluster.
            for (int j = i + 1; j < myDimIntersections; j++) {
                if (!myclustered[j]) {
                    double dist = distance(myIntersections[i].x, myIntersections[i].y, myIntersections[j].x, myIntersections[j].y);
                    if (dist <  THRESHOLD) {
                        cluster[clusterSize++] = myIntersections[j]; // Ajout du point au cluster.
                        myclustered[j] = 1;                           // Marquage du point comme regroupé.
                    }
                }
            }
            IntersectionPoint centroid = calculateCentroid(cluster, clusterSize);
            arrayCentroid[fillCounter++] = centroid;
        }
    }
}
