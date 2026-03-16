#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "vector.h"
#include "player.h"
#include "map.h"
#include "enemy.h"

// Işın izleme değişkenlerini tutan yapı
typedef struct
{
    VectorDouble rayDir;
    VectorDouble deltaDist;
    VectorDouble sideDist;
    int mapX;
    int mapY;
    int stepX;
    int stepY;
    int hit;
    int side;
    float perpWallDist;
} rayCasting;

// Fonksiyon prototipleri
void DDA(rayCasting *ray);
void renderFrame(character *player); // main.c içindeki dev for döngüsünü bu yapacak

#endif // RAYCASTER_H