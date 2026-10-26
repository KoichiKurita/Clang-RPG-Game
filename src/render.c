#include "render.h"

#include <stdarg.h>
#include <stdio.h>

#include "game.h"

#define COLOR_RESET  "\x1b[0m"
#define COLOR_PLAYER "\x1b[33m"   /* 黄 */
#define COLOR_ENEMY  "\x1b[31m"   /* 赤 */
#define COLOR_GOAL   "\x1b[32m"   /* 緑 */
#define COLOR_WALL   "\x1b[90m"   /* 灰 */
#define CLEAR_EOL    "\x1b[K"     /* 行末まで消去（前フレームの残りを消す） */

/* バッファへの安全な追記。容量を超えたら以降は書かずに切り詰める */
typedef struct {
    char  *buf;
    size_t cap;
    size_t len;
} Out;

static void out_printf(Out *o, const char *fmt, ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;

static void out_printf(Out *o, const char *fmt, ...) {
    if (o->len + 1 >= o->cap) return;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(o->buf + o->len, o->cap - o->len, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    size_t room = o->cap - o->len - 1;
    o->len += ((size_t)n < room) ? (size_t)n : room;
}

static void draw_cell(Out *o, const World *w, int x, int y) {
    Entity e = world_entity_at(w, x, y);
    if (e != ENTITY_NONE) {
        if (world_has(w, e, COMP_PLAYER)) { out_printf(o, COLOR_PLAYER "@" COLOR_RESET); return; }
        if (world_has(w, e, COMP_ENEMY))  { out_printf(o, COLOR_ENEMY  "E" COLOR_RESET); return; }
    }
    char t = w->map.tiles[y][x];
    switch (t) {
    case TILE_GOAL: out_printf(o, COLOR_GOAL "G" COLOR_RESET); break;
    case TILE_WALL: out_printf(o, COLOR_WALL "#" COLOR_RESET); break;
    default:        out_printf(o, "%c", t);                     break;
    }
}

size_t render_frame(const World *w, char *buf, size_t cap) {
    if (cap == 0) return 0;
    Out o = { buf, cap, 0 };
    buf[0] = '\0';

    out_printf(&o, "\x1b[H");   /* カーソルを左上へ（全消去しないのでちらつかない） */

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) draw_cell(&o, w, x, y);
        out_printf(&o, CLEAR_EOL "\n");
    }

    const CHealth *hp = &w->health[w->player];
    out_printf(&o, "\nHP %2d/%2d  Turn %4d  Enemies %d  Seed %lu" CLEAR_EOL "\n",
               hp->hp, hp->max_hp, w->turn, game_enemies_alive(w),
               (unsigned long)w->seed);
    out_printf(&o, "[WASD/Arrows] move/attack  [Space] wait  [P] save  [Q] quit" CLEAR_EOL "\n");
    out_printf(&o, "--- LOG ---" CLEAR_EOL "\n");
    for (int i = LOG_LINES - 1; i >= 0; i--) out_printf(&o, "%s" CLEAR_EOL "\n", w->log[i]);

    return o.len;
}