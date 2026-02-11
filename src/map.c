#include "map.h"
#include <stdlib.h>

void generate_map(char map[MAP_HEIGHT][MAP_WIDTH]) {
    // 初期化：全部壁
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = '#';
        }
    }

    // 内部をランダムに通路化
    for (int y = 1; y < MAP_HEIGHT - 1; y++) {
        for (int x = 1; x < MAP_WIDTH - 1; x++) {
            map[y][x] = (rand() % 100 < 70) ? '.' : '#';
        }
    }

    // スタート地点は必ず空ける
    map[1][1] = '.';

    // ゴール配置（右下付近）
    int gx = MAP_WIDTH - 2;
    int gy = MAP_HEIGHT - 2;
    map[gy][gx] = 'G';

    // ゴールまでの安全な通路を保証
    int x = 1, y = 1;
    while (x < gx) {
        map[y][x] = '.';
        x++;
    }
    while (y < gy) {
        map[y][x] = '.';
        y++;
    }
}
