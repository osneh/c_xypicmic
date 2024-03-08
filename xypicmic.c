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

// Structures pour stocker les coordonnées et les points d'intersection.
typedef struct {
    int row;
    int col;
} RowCol;

typedef struct {
    double x_start;
    double y_start;
    double x_end;
    double y_end;
} LineCoordinates;

typedef struct {
    double x;
    double y;
    bool intersects;
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
    double pitch = 5;
    double Ymax = 852 * 7.5;
    double Xmax = (852 * 7.5) * (sqrt(3)) / 3 * 2;

    if (lineType == 'Y') {
        coords.y_start = coords.y_end = value * 7.5;
    } else if (lineType == 'R') {
        double angle = -120; // 120 degrés en sens négatif pour la pente R
        coords.y_start = tan_deg(angle) * coords.x_start + (value - 852) * pitch + Ymax / 2;
        coords.y_end = tan_deg(angle) * coords.x_end + (value - 852) * pitch + Ymax / 2;
    } else if (lineType == 'B') {
        double angle = 120; // 120 degrés pour la pente B
        coords.y_start = tan_deg(angle) * coords.x_start + value * pitch + Ymax / 2;
        coords.y_end = tan_deg(angle) * coords.x_end + value * pitch + Ymax / 2;
    }

    // Les coordonnées x_start et x_end restent inchangées car elles représentent les extrémités du plan.
    coords.x_start = 0;
    coords.x_end = Xmax;

    return coords;
}

// Fonction pour calculer le point d'intersection entre deux lignes, si existant.
IntersectionPoint calculateIntersection(LineCoordinates line1, LineCoordinates line2) {
    IntersectionPoint result = {0, 0, false};
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
    if ((x >= fmin(line1.x_start, line1.x_end) && x <= fmax(line1.x_start, line1.x_end)) &&
        (y >= fmin(line1.y_start, line1.y_end) && y <= fmax(line1.y_start, line1.y_end)) &&
        (x >= fmin(line2.x_start, line2.x_end) && x <= fmax(line2.x_start, line2.x_end)) &&
        (y >= fmin(line2.y_start, line2.y_end) && y <= fmax(line2.y_start, line2.y_end))) {
        result.x = x;
        result.y = y;
        result.intersects = true;
    }

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


    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return -1;
    }
    replaceBackslashes(cwd); 

    
    char fileTablePath[PATH_MAX];
    snprintf(fileTablePath, sizeof(fileTablePath), "%s/picmic_adress_table.tab", cwd);

    
    char arr[ROWS][COLS][MAX_NAME_LENGTH] = {0};
    RowCol getPisteRowCol[1000];

    
    FILE *file = fopen(fileTablePath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char name[MAX_NAME_LENGTH], RYBi[MAX_NAME_LENGTH];
        int col, row;
        sscanf(line, "%d\t%d\t%*d\t%s", &col, &row, name); 

        extractRYBi(name, RYBi); 
        strcpy(arr[row][col], RYBi); 
        getPisteRowCol[atoi(RYBi)] = (RowCol){.row = row, .col = col}; 
    }
    fclose(file); 

    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char filename[64];
    strftime(filename, sizeof(filename), "coordinates_%Y%m%d_%H%M%S.csv", t);


    FILE *csvFile = fopen(filename, "w");
    if (csvFile == NULL) {
        perror("Error opening CSV file");
        return -1;
    }

    fprintf(csvFile, "Row,Col,X_start,Y_start,X_end,Y_end\n"); 


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
                LineCoordinates coords = calculateLineCoordinates(lineType, lineValue); // Calcul des coordonnées de la ligne.

                // Affichage des informations de la ligne et écriture dans le fichier CSV.
                printf("For Row %d, Column %d: Line Type: %c, Start (x,y): (%lf, %lf), End (x,y): (%lf, %lf)\n",
                       userInputRow, userInputCol, lineType, coords.x_start, coords.y_start, coords.x_end, coords.y_end);

                fprintf(csvFile, "%d,%d,%lf,%lf,%lf,%lf\n",
                        userInputRow, userInputCol,
                        coords.x_start, coords.y_start,
                        coords.x_end, coords.y_end);
            }
        } else {
            printf("Invalid row or column for element %d. Please enter values within the range.\n", i + 1);
        }
    }
    fclose(csvFile); 


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
                fprintf(intersectionsFile, "%d,%d,%d,%d,%lf,%lf\n", row1, col1, row2, col2, intersection.x, intersection.y);
                intersectionPoints[intersectionCount++] = intersection;
            }
        }
    }

    // Clustering des points d'intersection basé sur un seuil de distance.
    double threshold = 100.0;

    int clustered[numElements * (numElements - 1) / 2]; // Tableau pour marquer les points déjà regroupés.
	
	for (int i = 0; i < numElements * (numElements - 1) / 2; i++) {
		clustered[i] = 0;
	}
    for (int i = 0; i < intersectionCount; i++) {
        if (!clustered[i]) {
            IntersectionPoint cluster[numElements * (numElements - 1) / 2]; // Tableau temporaire pour stocker un cluster de points.
            int clusterSize = 0; // Taille du cluster.
            cluster[clusterSize++] = intersectionPoints[i]; // Ajout du point initial au cluster.
            clustered[i] = 1; // Marquage du point comme regroupé.

            // Recherche d'autres points à inclure dans le cluster.
            for (int j = i + 1; j < intersectionCount; j++) {
                if (!clustered[j]) {
                    double dist = distance(intersectionPoints[i].x, intersectionPoints[i].y, intersectionPoints[j].x, intersectionPoints[j].y);
                    if (dist < threshold) {
                        cluster[clusterSize++] = intersectionPoints[j]; // Ajout du point au cluster.
                        clustered[j] = 1; // Marquage du point comme regroupé.
                    }
                }
            }

            // Calcul du centroïde du cluster.
            IntersectionPoint centroid = calculateCentroid(cluster, clusterSize);
            // Écriture du centroïde dans le fichier CSV des intersections.
            fprintf(intersectionsFile, "Centroid,%lf,%lf\n", centroid.x, centroid.y);
        }
    }

    fclose(intersectionsFile); 

    return 0; 
}
