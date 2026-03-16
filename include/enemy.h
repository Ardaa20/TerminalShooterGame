#ifndef ENEMY_H
#define ENEMY_H

#include "vector.h"
#include <stdlib.h> // abs() fonksiyonu için gerekli
#include "map.h"
#include "player.h"

// Kaç tane düşman olacağını belirle
#define NUM_ENEMYS 3

// Düşman yapısı
typedef struct
{
    VectorDouble position; // Düşmanın haritadaki konumu
    double distance;       // Oyuncuya uzaklığı (sıralama için)
    char symbol;           // Ekranda basılacak karakter
} Enemy;

// Haritadaki düşmanların başlangıç pozisyonları
extern Enemy enemys[NUM_ENEMYS];
extern double ZBuffer[WIDTH]; // Her sütun için duvara olan uzaklığı tutan z-buffer

void sortEnemys(int amount);
void drawEnemys(character *player);

// Z-Buffer: Her X sütunundaki duvarın uzaklığını tutacak dizi
#endif // ENEMY_H