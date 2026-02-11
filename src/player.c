#include "player.h"
#include "map.h"   // MAP_WIDTH / MAP_HEIGHT を使うため

void init_player(Player* p) {
    p->x = 1;
    p->y = 1;
    p->max_hp = 20;
    p->hp = 20;
    p->atk = 4;
}

void move_player(Player* p, int dx, int dy, char map[MAP_HEIGHT][MAP_WIDTH]) {
    int nx = p->x + dx;
    int ny = p->y + dy;

    // ✅ 範囲チェック（超重要）
    if (nx < 0 || ny < 0 || nx >= MAP_WIDTH || ny >= MAP_HEIGHT)
        return;

    // ✅ 壁チェック
    if (map[ny][nx] == '#')
        return;

    p->x = nx;
    p->y = ny;
}
