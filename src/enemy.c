#include "enemy.h"
#include <stdlib.h>

static int in_view(Enemy* e, int x, int y) {
    return abs(e->x - x) <= 3 && abs(e->y - y) <= 3;
}

void init_enemy(Enemy* e, int x, int y) {
    e->x = e->ox = x;
    e->y = e->oy = y;
    e->hp = 8;
    e->state = ENEMY_IDLE;
    e->alive = 1;
}

void update_enemy(Enemy* e, int px, int py, char map[][40]) {
    if (!e->alive) return;

    switch (e->state) {
    case ENEMY_IDLE:
        if (in_view(e, px, py)) e->state = ENEMY_CHASE;
        break;
    case ENEMY_CHASE:
        e->x += (px > e->x) - (px < e->x);
        e->y += (py > e->y) - (py < e->y);
        if (!in_view(e, px, py)) e->state = ENEMY_RETURN;
        break;
    case ENEMY_RETURN:
        e->x += (e->ox > e->x) - (e->ox < e->x);
        e->y += (e->oy > e->y) - (e->oy < e->y);
        if (e->x == e->ox && e->y == e->oy)
            e->state = ENEMY_IDLE;
        break;
    }
}
