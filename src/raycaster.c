#include <math.h>
#include <ncurses.h>
#include "raycaster.h"

void DDA(rayCasting *ray)
{
    while (ray->hit == 0)
    {
        if (ray->sideDist.x < ray->sideDist.y)
        {
            ray->sideDist.x += ray->deltaDist.x;
            ray->mapX += ray->stepX;
            ray->side = 0;
        }
        else
        {
            ray->sideDist.y += ray->deltaDist.y;
            ray->mapY += ray->stepY;
            ray->side = 1;
        }
        // Check if ray has hit a wall
        if (map[ray->mapY][ray->mapX] > 0)
            ray->hit = 1;
    }
}

void renderFrame(character *player)
{
    rayCasting ray;

    for (int x = 0; x < WIDTH; x++)
    {
        double cameraX = 2 * x / (double)WIDTH - 1; // x-coordinate in camera space
        ray.rayDir.x = player->dir.x + player->plane.x * cameraX;
        ray.rayDir.y = player->dir.y + player->plane.y * cameraX;

        ray.mapX = (int)player->position.x;
        ray.mapY = (int)player->position.y;
        ray.deltaDist.x = (ray.rayDir.x == 0) ? 1e30 : fabs(1 / ray.rayDir.x);
        ray.deltaDist.y = (ray.rayDir.y == 0) ? 1e30 : fabs(1 / ray.rayDir.y);
        ray.hit = 0;

        if (ray.rayDir.x < 0)
        {
            ray.stepX = -1;
            ray.sideDist.x = (player->position.x - ray.mapX) * ray.deltaDist.x;
        }
        else
        {
            ray.stepX = 1;
            ray.sideDist.x = (ray.mapX + 1.0 - player->position.x) * ray.deltaDist.x;
        }

        if (ray.rayDir.y < 0)
        {
            ray.stepY = -1;
            ray.sideDist.y = (player->position.y - ray.mapY) * ray.deltaDist.y;
        }
        else
        {
            ray.stepY = 1;
            ray.sideDist.y = (ray.mapY + 1.0 - player->position.y) * ray.deltaDist.y;
        }

        DDA(&ray);

        // fish-eye etkisini önlemek için duvara olan gerçek uzaklığı hesapla
        if (ray.side == 0)
            ray.perpWallDist = (ray.sideDist.x - ray.deltaDist.x);
        else
            ray.perpWallDist = (ray.sideDist.y - ray.deltaDist.y);

        ZBuffer[x] = ray.perpWallDist; // o sütun için duvara olan uzaklığı z-buffer'a kaydet

        printMap(ray.perpWallDist, ray.side, x);
    }

    drawEnemys(player);
}
