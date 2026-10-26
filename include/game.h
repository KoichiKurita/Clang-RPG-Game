#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#include "world.h"

/* 入力を「意味」に変換したもの。キー配置の変更はここだけで済む */
typedef enum {
    CMD_NONE = 0,
    CMD_UP, CMD_DOWN, CMD_LEFT, CMD_RIGHT,
    CMD_WAIT,
    CMD_SAVE,
    CMD_QUIT
} Command;

/* seed から新しいゲームを作る（マップ生成・プレイヤーと敵の配置） */
void    game_new(World *w, uint32_t seed);

Command game_command_from_key(int key);

/* 1コマンドを処理する。ターンが進んだら 1、進まなかったら 0 */
int     game_step(World *w, Command cmd);

int     game_enemies_alive(const World *w);

#endif