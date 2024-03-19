#ifndef XYPICMIC_H
#define XYPICMIC_H

#include <stdbool.h>

#define COLS 54
#define ROWS 128
#define MAX_LINE_LENGTH 100
#define MAX_NAME_LENGTH 20
#define PI 3.14159265358979323846
#define THRESHOLD 100.0 

//extern char myarray [ROWS][COLS][MAX_NAME_LENGTH];

extern char arr[ROWS][COLS][MAX_NAME_LENGTH];

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
} IntersectionPoint;

void replaceBackslashes(char *str);
double distance(double , double , double , double ); 
void extractRYBi(const char *, char *);
LineCoordinates calculateLineCoordinates(char , int );
IntersectionPoint calculateIntersection(LineCoordinates line1, LineCoordinates line2);
IntersectionPoint calculateCentroid(IntersectionPoint *cluster, int size);
void splitLineColor(LineCoordinates *, int ,LineCoordinates *, LineCoordinates *, LineCoordinates *); 
void xLines(IntersectionPoint *, int ,LineCoordinates *, int , LineCoordinates *, int , LineCoordinates * , int , int * );
void fillCentroids(IntersectionPoint *, int , IntersectionPoint * , int  );
void fillLines(char * [],LineCoordinates *, int, int *, int *, int *);

#endif /* XYPICMIC_H */
