#include "systems.h"
#include "components.h"
#include "map.h"

void system_movement(Entity e, int dx, int dy, char map[MAP_HEIGHT][MAP_WIDTH]) {
    if (!has_position[e]) return;

    int nx = c_position[e].x + dx;
    int ny = c_position[e].y + dy;

    if (nx < 0 || ny < 0 || nx >= MAP_WIDTH || ny >= MAP_HEIGHT) return;
    if (map[ny][nx] == '#') return;

    c_position[e].x = nx;
    c_position[e].y = ny;
}
