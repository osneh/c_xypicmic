#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>

#define COLS 54
#define ROWS 128
#define MAX_LINE_LENGTH 100
#define MAX_NAME_LENGTH 20
#define PI 3.14159265358979323846
#define THRESHOLD 100.0 

typedef struct {
    double x_start;
    double y_start;
    double x_end;
    double y_end;
    char type;
    unsigned int val;
} LineCoordinates;

typedef struct {
    double x;
    double y;
    bool intersects;
    //unsigned short flag;
} IntersectionPoint;

double tan_deg(double angle) {
    return tan(angle * PI / 180.0);
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

// Fonction pour calculer les coordonnées d'une ligne basée sur son type et sa valeur.
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

// Fonction pour calculer le point d'intersection entre deux lignes, si existant.
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

    // Vérifie si le point d'intersection se trouve sur les segments de ligne.
    /*if ((x >= fmin(line1.x_start, line1.x_end) && x <= fmax(line1.x_start, line1.x_end)) &&
        (y >= fmin(line1.y_start, line1.y_end) && y <= fmax(line1.y_start, line1.y_end)) &&
        (x >= fmin(line2.x_start, line2.x_end) && x <= fmax(line2.x_start, line2.x_end)) &&
        (y >= fmin(line2.y_start, line2.y_end) && y <= fmax(line2.y_start, line2.y_end))) {
    */    result.x = x;
        result.y = y;
        result.intersects = true;
    //}

    return result;
}

void replaceBackslashes(char *str) {
    while (*str) {
        if (*str == '\\') *str = '/';
        str++;
    }
}

// Fonction pour calculer la distance entre deux points.
double distance(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Fonction pour calculer le centroïde d'un ensemble de points d'intersection.
IntersectionPoint calculateCentroid(IntersectionPoint *cluster, int size) {
    IntersectionPoint centroid = {0, 0, false};
    for (int i = 0; i < size; i++) {
        centroid.x += cluster[i].x;
        centroid.y += cluster[i].y;
    }
    centroid.x /= size;
    centroid.y /= size;
    return centroid;
}

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
        //getPisteRowCol[atoi(RYBi)] = (RowCol){.row = row, .col = col};    --> useless
    }
    fclose(file); 

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // compute lines according color
    for (int i = 0; i < numElements; i++) {
        int userInputRow = atoi(argv[2 + i * 2]); 
        int userInputCol = atoi(argv[3 + i * 2]); 

        if (userInputRow >= 0 && userInputRow < ROWS && userInputCol >= 0 && userInputCol < COLS) {
            char *value = arr[userInputRow][userInputCol]; 
            char lineType = value[0]; 

            // Traitement en fonction du type de ligne.
            if (lineType == 'D') {
                printf("For Row %d, Column %d: This is a dummy cell.\n", userInputRow, userInputCol);
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
    //fclose(csvFile); 

    // --------------------------
    // Arrays allocation (according her Line Color)
    // --------------------------
    LineCoordinates *ylines = (LineCoordinates *)malloc(y_size * sizeof(LineCoordinates));
    LineCoordinates *rlines = (LineCoordinates *)malloc(r_size * sizeof(LineCoordinates));
    LineCoordinates *blines = (LineCoordinates *)malloc(b_size * sizeof(LineCoordinates));

    int y_idx = 0;int r_idx = 0;int b_idx = 0;
    // -----------------------------
    for ( int j = 0 ; j < numElements; ++j){
        char ltype =  lineInEvent[j].type ;
        printf("%c%d\n",ltype,lineInEvent[j].val);
        if ( ltype == 'Y') { 
            ylines[y_idx] = lineInEvent[j];
            y_idx+=1;
        }
        else if ( ltype == 'R' ) {
            rlines[r_idx] = lineInEvent[j];
            r_idx+=1;    
        }
        else if ( ltype == 'B' ) {
            blines[b_idx] = lineInEvent[j];
            b_idx+=1;
        }
    }
    // --------------------------
    printf("HERE0\n");
    char intersectionsFilename[64];
    strftime(intersectionsFilename, sizeof(intersectionsFilename), "intersections_%Y%m%d_%H%M%S.csv", t);

    FILE *intersectionsFile = fopen(intersectionsFilename, "w");
    if (intersectionsFile == NULL) {
        perror("Error opening intersections file");
        return -1;
    }
    fprintf(intersectionsFile, "Line1_Row,Line1_Col,Line2_Row,Line2_Col,Intersection_X,Intersection_Y\n"); // En-tête du fichier CSV.

    IntersectionPoint intersectionPoints[numElements * (numElements - 1) / 2]; // Tableau pour stocker les points d'intersection.
    int intersectionCount = 0; 

    for (int i = 0; i < numElements; i++) {
        for (int j = i + 1; j < numElements; j++) {
            int row1 = atoi(argv[2 + i * 2]); 
            int col1 = atoi(argv[3 + i * 2]); 
            int row2 = atoi(argv[2 + j * 2]); 
            int col2 = atoi(argv[3 + j * 2]); 

            // Calcul des coordonnées des deux lignes.
            LineCoordinates line1 = calculateLineCoordinates(arr[row1][col1][0], atoi(&arr[row1][col1][1]));
            LineCoordinates line2 = calculateLineCoordinates(arr[row2][col2][0], atoi(&arr[row2][col2][1]));

            // Calcul du point d'intersection entre les deux lignes.
            IntersectionPoint intersection = calculateIntersection(line1, line2);

            // Si un point d'intersection est trouvé, il est enregistré et écrit dans le fichier CSV.
            if (intersection.intersects) {
                fprintf(intersectionsFile, "%d,%d,%d,%d,%0.2lf,%0.2lf\n", row1, col1, row2, col2, intersection.x, intersection.y);
                intersectionPoints[intersectionCount++] = intersection;
            }
        }
    }

    // ----------------------------------------------------
    //printf("HERE1\n");
    printf("ysize=%d,rsize=%d,bsize=%d\n",y_size,r_size,b_size);
    
    IntersectionPoint intersections[y_size*r_size + y_size*b_size + b_size*r_size ];
    int interCount = 0;

        if (y_size>0){
            for (int idx = 0 ; idx<y_size; idx++){              // Yellow lines Loop
                for (int jdx = 0 ; jdx<r_size; jdx++){          // Red lines Loop 
                    IntersectionPoint intersection = calculateIntersection(ylines[idx],rlines[jdx]);
                    //if (intersection.intersects) {
                    intersections[interCount++] = intersection; //printf("Y%d-R%d\n",ylines[idx].val,rlines[jdx].val);//}
                }

                for (int kdx = 0 ; kdx<b_size; kdx++){          // Blue lines Loop 
                    IntersectionPoint intersection = calculateIntersection(ylines[idx],blines[kdx]);
                    //if (intersection.intersects) {
                    intersections[interCount++] = intersection; //printf("Y%d-B%d\n",ylines[idx].val,blines[kdx].val);//}
                }
            }
        }

        if (r_size>0 && b_size > 0 ) {              // Red and Blue lines Loop
            //printf("START\n");
            for (int idx = 0; idx<r_size; idx++){
                for (int jdx = 0 ; jdx<b_size; jdx++){
                    IntersectionPoint intersection = calculateIntersection(rlines[idx],blines[jdx]);
                    //if (intersection.intersects) {
                    intersections[interCount++] = intersection; //printf("R%d-B%d\n",rlines[idx].val,blines[jdx].val);//}
                }
            }
        }
    //printf("HERE10\n");
    //return 0;
    // -----------------------------------------------------
    
    printf("intercount=%d\n",interCount);
    for ( int z = 0 ; z<interCount; z++){
        //fprintf("z=%d,x=%0.2lf,y=%0.2lf\n", intersections[z].x, intersections[z].y);
        printf("z=%d,x=%0.2f,y=%0.2f\n", z,intersections[z].x, intersections[z].y);
        //printf("%d\n",z);
    }
    
    // ---------------------------------------------
    //return 0;
    // Clustering des points d'intersection basé sur un seuil de distance.
    //double threshold = 100.0;

    //int clustered[numElements * (numElements - 1) / 2]={0}; // Tableau pour marquer les points déjà regroupés.
	int clustered[interCount];
	memset(clustered,0,interCount*sizeof(int));

    for (int i = 0; i < interCount; i++) { //printf("HERE12\n");
        if (!clustered[i]) {
            IntersectionPoint cluster[interCount]; // Tableau temporaire pour stocker un cluster de points.
            int clusterSize = 0; // Taille du cluster.
            cluster[clusterSize++] = intersections[i]; // Ajout du point initial au cluster.
            clustered[i] = 1; // Marquage du point comme regroupé.
            //printf("HERE13\n");
            // Recherche d'autres points à inclure dans le cluster.
            for (int j = i + 1; j < intersectionCount; j++) {
                if (!clustered[j]) {
                    double dist = distance(intersections[i].x, intersections[i].y, intersections[j].x, intersections[j].y);
                    if (dist <  THRESHOLD) {
                        cluster[clusterSize++] = intersections[j]; // Ajout du point au cluster.
                        clustered[j] = 1; // Marquage du point comme regroupé.
                    }
                }
            }
            //printf("HERE14\n");
            // Calcul du centroïde du cluster.
            IntersectionPoint centroid = calculateCentroid(cluster, clusterSize);
            // Écriture du centroïde dans le fichier CSV des intersections.
            fprintf(intersectionsFile, "Centroid,%0.2lf,%0.2lf\n", centroid.x, centroid.y);
        }
    }

    fclose(intersectionsFile); 


    // ---- 
    free(ylines);
    free(rlines);
    free(blines);
    //free(intersections);

    return 0; 
}