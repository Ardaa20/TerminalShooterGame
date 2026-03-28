#include "player.h"
#include "map.h"

void movePlayer(character *player, int key)
{
    Vector nextPlayerPosition = player->position;
    if (key == 'w')
    {
        nextPlayerPosition.x += PLAYERSPEED * player->dir.x;
        nextPlayerPosition.y += PLAYERSPEED * player->dir.y;
    }
    if (key == 's')
    {
        nextPlayerPosition.x -= PLAYERSPEED * player->dir.x;
        nextPlayerPosition.y -= PLAYERSPEED * player->dir.y;
    }
    if (key == 'a')
    {
        nextPlayerPosition.x += PLAYERSPEED * player->dir.y;
        nextPlayerPosition.y -= PLAYERSPEED * player->dir.x;
    }
    if (key == 'd')
    {
        nextPlayerPosition.x -= PLAYERSPEED * player->dir.y;
        nextPlayerPosition.y += PLAYERSPEED * player->dir.x;
    }
    if (!isWall(nextPlayerPosition))
    {
        player->position = nextPlayerPosition;
    }
    return;
}

void rotatePlayer(character *player, int deltaX)
{
    // Rotation matrix kullanarak oyuncunun yönünü ve kamera düzlemini döndürüyoruz
    double rotSpeed = deltaX * ROTSPEED;
    double oldDirX = player->dir.x;
    player->dir.x = player->dir.x * cos(rotSpeed) - player->dir.y * sin(rotSpeed);
    player->dir.y = oldDirX * sin(rotSpeed) + player->dir.y * cos(rotSpeed);

    double oldPlaneX = player->plane.x;
    player->plane.x = player->plane.x * cos(rotSpeed) - player->plane.y * sin(rotSpeed);
    player->plane.y = oldPlaneX * sin(rotSpeed) + player->plane.y * cos(rotSpeed);
}

/*
    if (map[(int)nextPlayerPosition.y][(int)nextPlayerPosition.x] == 0)
    {
        player.position.x = nextPlayerPosition.x;
        player.position.y = nextPlayerPosition.y;
    }
    nextPlayerPosition.x = player.position.x;
    nextPlayerPosition.y = player.position.y;
*/