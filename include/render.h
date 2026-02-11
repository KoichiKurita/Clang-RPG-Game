#ifndef RENDER_H
#define RENDER_H

#include "ecs.h"
#include "map.h"

void render(
    char map[MAP_HEIGHT][MAP_WIDTH],
    Entity player,
    Entity enemies[],
    int enemy_count
);

#endif
