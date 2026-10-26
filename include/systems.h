#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "world.h"

typedef enum {
    MOVE_BLOCKED = 0,   /* 壁・範囲外・味方がいて動けない */
    MOVE_MOVED,         /* 移動した */
    MOVE_ATTACKED       /* 移動先の相手を攻撃した（体当たり攻撃） */
} MoveResult;

/* 移動 System：移動先に敵対エンティティがいれば攻撃に切り替える */
MoveResult system_move(World *w, Entity e, int dx, int dy);

/* 戦闘 System：attacker の攻撃力ぶん target の HP を減らす */
void system_attack(World *w, Entity attacker, Entity target);

/* 敵AI System：全ての敵を1手ずつ動かす（IDLE → CHASE → RETURN） */
void system_enemy_ai(World *w);

#endif
