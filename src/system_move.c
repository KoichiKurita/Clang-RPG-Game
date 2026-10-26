#include "systems.h"

/* プレイヤーと敵は敵対、敵同士は味方 */
static int is_hostile(const World *w, Entity a, Entity b) {
    int a_player = world_has(w, a, COMP_PLAYER);
    int b_player = world_has(w, b, COMP_PLAYER);
    return a_player != b_player;
}

MoveResult system_move(World *w, Entity e, int dx, int dy) {
    if (!world_has(w, e, COMP_POSITION)) return MOVE_BLOCKED;

    int nx = w->pos[e].x + dx;
    int ny = w->pos[e].y + dy;

    if (!map_is_walkable(&w->map, nx, ny)) return MOVE_BLOCKED;

    Entity other = world_entity_at(w, nx, ny);
    if (other != ENTITY_NONE) {
        if (is_hostile(w, e, other) && world_has(w, e, COMP_ATTACK)) {
            system_attack(w, e, other);
            return MOVE_ATTACKED;
        }
        return MOVE_BLOCKED;   /* 味方とは重ならない */
    }

    w->pos[e].x = nx;
    w->pos[e].y = ny;
    return MOVE_MOVED;
}
