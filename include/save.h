#ifndef SAVE_H
#define SAVE_H

#include "player.h"
#include "enemy.h"
#include "map.h"
#include "game_state.h"

#define SAVE_FILE "save.dat"

/* セーブ */
int save_game(
    Player* player,
    Enemy enemies[],
    int enemy_count,
    char map[MAP_HEIGHT][MAP_WIDTH],
    GameState* game
);

/* ロード */
int load_game(
    Player* player,
    Enemy enemies[],
    int enemy_count,
    char map[MAP_HEIGHT][MAP_WIDTH],
    GameState* game
);

#endif
