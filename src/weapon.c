#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "weapon.h"
#include "map.h" // WIDTH ve HEIGHT makrolarının burada tanımlı olduğunu varsayıyoruz

// Global değişkenlerin asıl tanımları (bellekte yer ayrılan yer)
Weapon weapons[MAXWEAPON];
int weaponCount = 0;
int shooting = 0;

void downloadWeapon()
{
    FILE *file = fopen("./weapon_image/weapon1.txt", "r");
    if (file == NULL)
    {
        printf("ERROR: FILE NOT FOUND!\n");
        return;
    }
    int width, height, tip;
    while (weaponCount < MAXWEAPON && fscanf(file, "w:%d h:%d t:%d", &width, &height, &tip) == 3)
    {
        int ch;
        while ((ch = fgetc(file)) != '\n' && ch != EOF)
            ;

        weapons[weaponCount].width = width;
        weapons[weaponCount].height = height;
        weapons[weaponCount].tip = tip;
        weapons[weaponCount].ascii_art = (char **)malloc(height * sizeof(char *));

        for (int i = 0; i < height; i++)
        {
            weapons[weaponCount].ascii_art[i] = (char *)malloc(width * sizeof(char));
            int j = 0;
            int current_char;
            while ((current_char = fgetc(file)) != '\n' && current_char != EOF)
            {
                if (current_char != '\r' && j < width)
                {
                    weapons[weaponCount].ascii_art[i][j] = current_char;
                    j++;
                }
            }
            while (j < width)
            {
                weapons[weaponCount].ascii_art[i][j] = ' ';
                j++;
            }
        }
        weaponCount++;
    }
    fclose(file);
}

void cleanMemory()
{
    for (int s = 0; s < weaponCount; s++)
    {
        int h = weapons[s].height;
        for (int i = 0; i < h; i++)
        {
            free(weapons[s].ascii_art[i]);
        }
        free(weapons[s].ascii_art);
    }
}

void printWeapon(Weapon *currentWeapon)
{
    if (weaponCount == 0)
        return;

    int w = currentWeapon->width;
    int h = currentWeapon->height;
    int offsetX = (COLS > WIDTH) ? (COLS - WIDTH) / 2 : 0;
    int offsetY = (LINES > HEIGHT) ? (LINES - HEIGHT) / 2 : 0;
    int widthStart = (WIDTH / 2) - (w / 2);
    int heightStart = HEIGHT - h;

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            char c = currentWeapon->ascii_art[i][j];
            if (c != ' ')
                mvaddch(heightStart + i + offsetY, widthStart + j + offsetX, c);
        }
    }
}

void shoot(Weapon *currentWeapon)
{

    int h = currentWeapon->height;
    int t = currentWeapon->tip;
    double wStart = WIDTH / 2 - t / 2;
    int offsetX = (COLS > WIDTH) ? (COLS - WIDTH) / 2 : 0;
    int offsetY = (LINES > HEIGHT) ? (LINES - HEIGHT) / 2 : 0;

    for (double m = wStart; m < wStart + t; m++)
    {
        for (int i = HEIGHT - h - 3; i < HEIGHT - h; i++)
        {
            mvaddch(i + offsetY, m + offsetX, '|');
        }
    }
}