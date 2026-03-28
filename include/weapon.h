#ifndef WEAPON_H
#define WEAPON_H

#define MAXWEAPON 10

// Silah yapısı (struct)
typedef struct
{
    int width;
    int height;
    int tip;
    char **ascii_art;
} Weapon;

// Global değişkenleri dışarıya açıyoruz (extern kullanarak)
extern Weapon weapons[MAXWEAPON];
extern int weaponCount;
extern int shooting;

// Fonksiyon prototipleri
void downloadWeapon(void);
void cleanMemory(void);
void printWeapon(Weapon *currentWeapon);
void shoot(Weapon *currentWeapon);

#endif // WEAPON_H