#include "render.h"
#include "components.h"
#include "log.h"
#include <stdio.h>

void render(
    char map[MAP_HEIGHT][MAP_WIDTH],
    Entity player,
    Entity enemies[],
    int enemy_count
) {
    printf("\x1b[H");

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {

            int drawn = 0;

            // プレイヤー
            if (has_position[player] &&
                c_position[player].x == x &&
                c_position[player].y == y) {
                printf("@");
                continue;
            }

            // 敵（複数）
            for (int i = 0; i < enemy_count; i++) {
                Entity e = enemies[i];
                if (has_position[e] &&
                    c_health[e].hp > 0 &&
                    c_position[e].x == x &&
                    c_position[e].y == y) {
                    printf("E");
                    drawn = 1;
                    break;
                }
            }
            if (drawn) continue;

            // マップ
            printf("%c", map[y][x]);
        }
        printf("\n");
    }

    // HP表示
    printf("\nHP %d / %d\n",
        c_health[player].hp,
        c_health[player].max_hp
    );

    // ログ表示
    log_draw();
}
