#ifndef MAP_H
#define MAP_H

#include "config.h"
#include "rng.h"

#define TILE_WALL  '#'
#define TILE_FLOOR '.'
#define TILE_GOAL  'G'

#define START_X 1
#define START_Y 1
#define GOAL_X  (MAP_WIDTH  - 2)
#define GOAL_Y  (MAP_HEIGHT - 2)

typedef struct {
    char tiles[MAP_HEIGHT][MAP_WIDTH];
} Map;

/* ランダムなマップを生成する。スタートからゴールまで必ず到達できる。 */
void map_generate(Map *m, Rng *rng);

int  map_in_bounds(int x, int y);
int  map_is_walkable(const Map *m, int x, int y);
int  map_is_valid_tile(char t);

/* 幅優先探索で (sx,sy) から (gx,gy) へ歩いて行けるか調べる */
int  map_reachable(const Map *m, int sx, int sy, int gx, int gy);

#endif