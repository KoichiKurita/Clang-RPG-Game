#include "world.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void world_init(World *w) {
    memset(w, 0, sizeof *w);
    w->player = ENTITY_NONE;
    w->status = GAME_PLAYING;
}

Entity world_create_entity(World *w) {
    if (w->entity_count >= MAX_ENTITIES) return ENTITY_NONE;   /* 上限チェック */
    Entity e = w->entity_count++;
    w->mask[e] = 0;
    return e;
}

void world_destroy_entity(World *w, Entity e) {
    if (world_is_valid(w, e)) w->mask[e] = 0;   /* Component を全部外す＝存在しない扱い */
}

int world_is_valid(const World *w, Entity e) {
    return e >= 0 && e < w->entity_count;
}

int world_has(const World *w, Entity e, uint32_t comps) {
    return world_is_valid(w, e) && (w->mask[e] & comps) == comps;
}

Entity world_entity_at(const World *w, int x, int y) {
    for (Entity e = 0; e < w->entity_count; e++) {
        if (world_has(w, e, COMP_POSITION | COMP_HEALTH) &&
            w->pos[e].x == x && w->pos[e].y == y) {
            return e;
        }
    }
    return ENTITY_NONE;
}

void world_log(World *w, const char *fmt, ...) {
    /* 古い行を1つずつ後ろへずらす（重なる領域なので memmove） */
    memmove(w->log[1], w->log[0], sizeof w->log[0] * (LOG_LINES - 1));

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(w->log[0], sizeof w->log[0], fmt, ap);   /* 長くても必ず NUL 終端 */
    va_end(ap);
}