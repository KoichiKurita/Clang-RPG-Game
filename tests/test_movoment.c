#include <assert.h>
#include "../include/ecs.h"
#include "../include/components.h"
#include "../include/systems.h"
#include "../include/map.h"

void test_player_move() {
    char map[MAP_HEIGHT][MAP_WIDTH];
    generate_map(map);

    Entity e = ecs_create();
    has_position[e] = 1;
    c_position[e].x = 1;
    c_position[e].y = 1;

    system_movement(e, 1, 0, map);

    assert(c_position[e].x == 2);
}

int main() {
    test_player_move();
    return 0;
}
