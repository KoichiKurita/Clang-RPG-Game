#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"

typedef struct {
    int x, y;
    int hp, max_hp;
    int atk;
} Player;

void init_player(Player* p);
void move_player(Player* p, int dx, int dy, char map[MAP_HEIGHT][MAP_WIDTH]);

#endif
