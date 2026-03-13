
#ifndef PLAYER_H
#define PLAYER_H

#include "vector.h"
#include <math.h>

#define ROTSPEED 0.07
#define PLAYERSPEED 0.05

typedef struct
{
    Vector position;
    Vector moveSpeed;
    Vector dir;
    Vector plane;
} character;

void movePlayer(character *player, int key);
void rotatePlayer(character *player, int deltaX);

#endif