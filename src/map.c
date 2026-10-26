#include "map.h"

#include <string.h>

#define WALL_PERCENT   30   /* 内部セルが壁になる確率 */
#define MAX_RETRY     100   /* 到達不能だった時の再生成回数 */

int map_in_bounds(int x, int y) {
    return x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT;
}

int map_is_walkable(const Map *m, int x, int y) {
    return map_in_bounds(x, y) && m->tiles[y][x] != TILE_WALL;
}

int map_is_valid_tile(char t) {
    return t == TILE_WALL || t == TILE_FLOOR || t == TILE_GOAL;
}

int map_reachable(const Map *m, int sx, int sy, int gx, int gy) {
    if (!map_is_walkable(m, sx, sy) || !map_is_walkable(m, gx, gy)) return 0;

    /* キューは固定長配列（全セル数を超えることはない） */
    static const int dx[4] = { 1, -1, 0, 0 };
    static const int dy[4] = { 0, 0, 1, -1 };
    int qx[MAP_WIDTH * MAP_HEIGHT];
    int qy[MAP_WIDTH * MAP_HEIGHT];
    unsigned char visited[MAP_HEIGHT][MAP_WIDTH];
    memset(visited, 0, sizeof visited);

    int head = 0, tail = 0;
    qx[tail] = sx; qy[tail] = sy; tail++;
    visited[sy][sx] = 1;

    while (head < tail) {
        int x = qx[head], y = qy[head]; head++;
        if (x == gx && y == gy) return 1;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i], ny = y + dy[i];
            if (map_is_walkable(m, nx, ny) && !visited[ny][nx]) {
                visited[ny][nx] = 1;
                qx[tail] = nx; qy[tail] = ny; tail++;
            }
        }
    }
    return 0;
}

static void fill_random(Map *m, Rng *rng) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int border = (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1);
            m->tiles[y][x] = (border || rng_range(rng, 100) < WALL_PERCENT)
                             ? TILE_WALL : TILE_FLOOR;
        }
    }
    m->tiles[START_Y][START_X] = TILE_FLOOR;
    m->tiles[GOAL_Y][GOAL_X]   = TILE_GOAL;
}

/* 最終手段：L字の通路を掘って到達を保証する */
static void carve_fallback_path(Map *m) {
    for (int x = START_X; x <= GOAL_X; x++) m->tiles[START_Y][x] = TILE_FLOOR;
    for (int y = START_Y; y <= GOAL_Y; y++) m->tiles[y][GOAL_X]  = TILE_FLOOR;
    m->tiles[GOAL_Y][GOAL_X] = TILE_GOAL;
}

void map_generate(Map *m, Rng *rng) {
    for (int i = 0; i < MAX_RETRY; i++) {
        fill_random(m, rng);
        if (map_reachable(m, START_X, START_Y, GOAL_X, GOAL_Y)) return;
    }
    carve_fallback_path(m);
}
