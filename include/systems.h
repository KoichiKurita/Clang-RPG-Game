#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "ecs.h"
#include "map.h"

/* 移動 */
void system_movement(
    Entity e,
    int dx,
    int dy,
    char map[MAP_HEIGHT][MAP_WIDTH]
);

/* 敵AI（追跡） */
void system_enemy_ai(
    Entity enemy,
    Entity player,
    char map[MAP_HEIGHT][MAP_WIDTH]
);

#endif
