#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

#include "config.h"
#include "map.h"
#include "rng.h"

/* ===== Entity ===== */
typedef int Entity;
#define ENTITY_NONE (-1)

/* ===== Component（データのみ。処理は System 側に書く） ===== */
typedef struct { int x, y; }       CPosition;
typedef struct { int hp, max_hp; } CHealth;
typedef struct { int atk; }        CAttack;

typedef enum { AI_IDLE = 0, AI_CHASE, AI_RETURN, AI_STATE_COUNT } AIState;
typedef struct { AIState state; int home_x, home_y; } CAI;

/* どの Component を持っているかをビットで表す */
enum {
    COMP_POSITION = 1u << 0,
    COMP_HEALTH   = 1u << 1,
    COMP_ATTACK   = 1u << 2,
    COMP_AI       = 1u << 3,
    COMP_PLAYER   = 1u << 4,   /* タグ：プレイヤー */
    COMP_ENEMY    = 1u << 5,   /* タグ：敵 */
    COMP_ALL      = (1u << 6) - 1u
};

typedef enum { GAME_PLAYING = 0, GAME_CLEAR, GAME_OVER, GAME_QUIT, GAME_STATUS_COUNT } GameStatus;

/* ===== World：ゲーム状態をすべて1つの構造体に集約 =====
 * グローバル変数を使わないため、テストでは World を何個でも作り直せる。 */
typedef struct {
    int       entity_count;
    uint32_t  mask[MAX_ENTITIES];
    CPosition pos[MAX_ENTITIES];
    CHealth   health[MAX_ENTITIES];
    CAttack   attack[MAX_ENTITIES];
    CAI       ai[MAX_ENTITIES];

    Entity     player;
    Map        map;
    Rng        rng;
    uint32_t   seed;
    int        turn;
    GameStatus status;

    char log[LOG_LINES][LOG_LEN];   /* log[0] が最新 */
} World;

void   world_init(World *w);
Entity world_create_entity(World *w);   /* 満杯なら ENTITY_NONE */
void   world_destroy_entity(World *w, Entity e);
int    world_is_valid(const World *w, Entity e);
int    world_has(const World *w, Entity e, uint32_t comps);

/* (x,y) にいる生存エンティティを返す。いなければ ENTITY_NONE */
Entity world_entity_at(const World *w, int x, int y);

/* printf 形式でログを追加（書式文字列は必ずリテラルで渡すこと） */
void world_log(World *w, const char *fmt, ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;

#endif