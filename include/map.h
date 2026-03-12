#ifndef MAP_H
#define MAP_H

#define HEIGHT 45
#define WIDTH 190

#define MAP_WIDTH 20
#define MAP_HEIGHT 20
#include "vector.h"

extern int map[MAP_HEIGHT][MAP_WIDTH];

int isWall(Vector pos);
void printMap(double wallDist, int side, int whichCol);

#endif