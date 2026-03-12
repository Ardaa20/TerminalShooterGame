#include "player.h"
#include "map.h"

Vector movePlayer(character *player, int key)
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
    if (isWall(nextPlayerPosition))
        return player->position;
    else
        return nextPlayerPosition;
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