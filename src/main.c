#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

#include "systems.h"
#include "ecs.h"
#include "components.h"
#include "systems.h"
#include "map.h"
#include "render.h"

#define MAX_ENEMIES 8

typedef enum {
    GAME_PLAYING,
    GAME_CLEAR
} GameStatus;

int main(void) {
    srand((unsigned)time(NULL));
    system("cls");

    char map[MAP_HEIGHT][MAP_WIDTH];
    generate_map(map);

    // ===== Player =====
    Entity player = ecs_create();
    has_position[player] = has_health[player] = has_attack[player] = has_tag[player] = 1;

    c_position[player] = (CPosition){1,1};
    c_health[player]   = (CHealth){20,20};
    c_attack[player]   = (CAttack){4};
    c_tag[player]      = (CTag){1};

    // ===== Enemies =====
    Entity enemies[MAX_ENEMIES];
    int enemy_count = 3;

    GameStatus status = GAME_PLAYING;

    for (int i = 0; i < enemy_count; i++) {
        enemies[i] = ecs_create();
        has_position[enemies[i]] = has_health[enemies[i]] = has_attack[enemies[i]] = 1;

        c_position[enemies[i]] = (CPosition){10 + i * 2, 10};
        c_health[enemies[i]]   = (CHealth){10,10};
        c_attack[enemies[i]]   = (CAttack){3};
    }

    // ===== Game Loop =====
    while (status == GAME_PLAYING) {

        render(map, player, enemies, enemy_count);

        if (_kbhit()) {
            char c = _getch();
            if (c=='w') system_movement(player, 0, -1, map);
            if (c=='s') system_movement(player, 0,  1, map);
            if (c=='a') system_movement(player,-1,  0, map);
            if (c=='d') system_movement(player, 1,  0, map);
            if (c=='q') break;
        }

        // 敵AI
        for (int i = 0; i < enemy_count; i++) {
            system_enemy_ai(enemies[i], player, map);
        }

        // 🎯 ゴール判定
        if (map[c_position[player].y][c_position[player].x] == 'G') {
            status = GAME_CLEAR;
        }

        Sleep(100);
    }

    system("cls");

    if (status == GAME_CLEAR) {
        printf("\n");
        printf("#############################\n");
        printf("#                           #\n");
        printf("#        🎉 GAME CLEAR 🎉     #\n");
        printf("#                           #\n");
        printf("#############################\n");
        printf("\n");
    } else {
        printf("GAME OVER\n");
    }
    
    return 0;
}
