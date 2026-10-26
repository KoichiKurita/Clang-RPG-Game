#include "systems.h"

#include <stdlib.h>

static int sign(int v) { return (v > 0) - (v < 0); }

static int distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);   /* 4方向移動なのでマンハッタン距離 */
}

/* 目標へ1歩近づく。距離の大きい軸を優先し、塞がれていたらもう一方の軸を試す */
static void step_toward(World *w, Entity e, int tx, int ty) {
    int dx = tx - w->pos[e].x;
    int dy = ty - w->pos[e].y;
    if (dx == 0 && dy == 0) return;

    if (abs(dx) >= abs(dy)) {
        if (dx != 0 && system_move(w, e, sign(dx), 0) != MOVE_BLOCKED) return;
        if (dy != 0) system_move(w, e, 0, sign(dy));
    } else {
        if (system_move(w, e, 0, sign(dy)) != MOVE_BLOCKED) return;
        if (dx != 0) system_move(w, e, sign(dx), 0);
    }
}

static void update_one(World *w, Entity e) {
    CAI *ai = &w->ai[e];
    Entity p = w->player;
    int d = distance(w->pos[e].x, w->pos[e].y, w->pos[p].x, w->pos[p].y);

    switch (ai->state) {
    case AI_IDLE:
        if (d <= ENEMY_VIEW) {
            ai->state = AI_CHASE;
            world_log(w, "An enemy noticed you!");
        }
        break;

    case AI_CHASE:
        if (d > ENEMY_LOSE) {
            ai->state = AI_RETURN;
            break;
        }
        step_toward(w, e, w->pos[p].x, w->pos[p].y);   /* 隣接していれば攻撃になる */
        break;

    case AI_RETURN:
        if (d <= ENEMY_VIEW) {
            ai->state = AI_CHASE;
            break;
        }
        step_toward(w, e, ai->home_x, ai->home_y);
        if (w->pos[e].x == ai->home_x && w->pos[e].y == ai->home_y) ai->state = AI_IDLE;
        break;

    default:
        ai->state = AI_IDLE;
        break;
    }
}

void system_enemy_ai(World *w) {
    if (!world_has(w, w->player, COMP_POSITION)) return;

    for (Entity e = 0; e < w->entity_count; e++) {
        if (w->status != GAME_PLAYING) return;   /* プレイヤーが倒れたら止める */
        if (world_has(w, e, COMP_ENEMY | COMP_AI | COMP_POSITION)) update_one(w, e);
    }
}