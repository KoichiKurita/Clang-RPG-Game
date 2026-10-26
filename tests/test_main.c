/*
 * ユニットテスト（外部ライブラリなしの最小構成）
 * 実行: make test
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "platform.h"
#include "render.h"
#include "save.h"
#include "systems.h"

/* ===== 最小テストフレームワーク ===== */
static int g_failed = 0;
static int g_checks = 0;

#define CHECK(cond) do {                                              \
        g_checks++;                                                   \
        if (!(cond)) {                                                \
            g_failed++;                                               \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
        }                                                             \
    } while (0)

#define RUN(test) do { printf("[RUN] %s\n", #test); test(); } while (0)

/* ===== テスト用ヘルパー ===== */

/* 外周だけ壁、内側は全部床のマップにプレイヤーを1人置いた World */
static void make_flat_world(World *w) {
    world_init(w);
    rng_seed(&w->rng, 1);
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++) {
            int border = (x == 0 || y == 0 || x == MAP_WIDTH - 1 || y == MAP_HEIGHT - 1);
            w->map.tiles[y][x] = border ? TILE_WALL : TILE_FLOOR;
        }
    Entity p = world_create_entity(w);
    w->player    = p;
    w->mask[p]   = COMP_POSITION | COMP_HEALTH | COMP_ATTACK | COMP_PLAYER;
    w->pos[p]    = (CPosition){ 5, 5 };
    w->health[p] = (CHealth){ PLAYER_HP, PLAYER_HP };
    w->attack[p] = (CAttack){ PLAYER_ATK };
}

static Entity add_enemy(World *w, int x, int y) {
    Entity e = world_create_entity(w);
    w->mask[e]   = COMP_POSITION | COMP_HEALTH | COMP_ATTACK | COMP_AI | COMP_ENEMY;
    w->pos[e]    = (CPosition){ x, y };
    w->health[e] = (CHealth){ ENEMY_HP, ENEMY_HP };
    w->attack[e] = (CAttack){ ENEMY_ATK };
    w->ai[e]     = (CAI){ AI_IDLE, x, y };
    return e;
}

/* セーブデータ内の u32 を書き換え、CRC を付け直す（「CRC は正しいが中身が不正」なデータを作る） */
static void patch_u32(uint8_t *buf, size_t len, size_t offset, uint32_t v) {
    for (int i = 0; i < 4; i++) buf[offset + (size_t)i] = (uint8_t)(v >> (8 * i));
    uint32_t crc = save_crc32(buf + SAVE_HEADER_SIZE, len - SAVE_HEADER_SIZE);
    for (int i = 0; i < 4; i++) buf[12 + i] = (uint8_t)(crc >> (8 * i));
}

/* Payload 内のオフセット（save.h のフォーマット定義と対応） */
#define OFF_TURN    (SAVE_HEADER_SIZE + 8)
#define OFF_STATUS  (SAVE_HEADER_SIZE + 12)
#define OFF_COUNT   (SAVE_HEADER_SIZE + 20)
#define OFF_ENTITY(i, field) (SAVE_HEADER_SIZE + SAVE_FIXED_SIZE + SAVE_ENTITY_SIZE * (i) + 4 * (field))
enum { F_MASK = 0, F_X, F_Y, F_HP, F_MAXHP, F_ATK, F_AI, F_HOMEX, F_HOMEY };

/* ===== RNG ===== */
static void test_rng_is_deterministic(void) {
    Rng a, b;
    rng_seed(&a, 42);
    rng_seed(&b, 42);
    for (int i = 0; i < 100; i++) CHECK(rng_next(&a) == rng_next(&b));

    rng_seed(&a, 0);   /* seed 0 でも 0 を返し続けない */
    CHECK(rng_next(&a) != 0);

    for (int i = 0; i < 1000; i++) {
        int v = rng_range(&a, 10);
        CHECK(v >= 0 && v < 10);
    }
    CHECK(rng_range(&a, 0) == 0);
}

/* ===== マップ ===== */
static void test_map_always_reachable(void) {
    static Map m;
    for (uint32_t seed = 1; seed <= 300; seed++) {
        Rng r;
        rng_seed(&r, seed);
        map_generate(&m, &r);
        CHECK(map_reachable(&m, START_X, START_Y, GOAL_X, GOAL_Y));
        CHECK(m.tiles[GOAL_Y][GOAL_X] == TILE_GOAL);
        for (int x = 0; x < MAP_WIDTH; x++) {   /* 外周は必ず壁 */
            CHECK(m.tiles[0][x] == TILE_WALL);
            CHECK(m.tiles[MAP_HEIGHT - 1][x] == TILE_WALL);
        }
    }
}

static void test_map_same_seed_same_map(void) {
    static Map a, b;
    Rng r1, r2;
    rng_seed(&r1, 123); map_generate(&a, &r1);
    rng_seed(&r2, 123); map_generate(&b, &r2);
    CHECK(memcmp(&a, &b, sizeof a) == 0);
}

/* ===== ECS ===== */
static void test_entity_limit(void) {
    static World w;
    world_init(&w);
    for (int i = 0; i < MAX_ENTITIES; i++) CHECK(world_create_entity(&w) != ENTITY_NONE);
    CHECK(world_create_entity(&w) == ENTITY_NONE);   /* 上限を超えたら作れない */
    CHECK(!world_is_valid(&w, -1));
    CHECK(!world_is_valid(&w, MAX_ENTITIES));
}

static void test_game_new_spawns_correctly(void) {
    static World w;
    for (uint32_t seed = 1; seed <= 50; seed++) {
        game_new(&w, seed);
        CHECK(w.pos[w.player].x == START_X && w.pos[w.player].y == START_Y);
        CHECK(game_enemies_alive(&w) == ENEMY_COUNT);
        for (Entity e = 0; e < w.entity_count; e++) {
            if (!world_has(&w, e, COMP_ENEMY)) continue;
            CHECK(w.map.tiles[w.pos[e].y][w.pos[e].x] == TILE_FLOOR);   /* 壁の上に出ない */
            CHECK(world_entity_at(&w, w.pos[e].x, w.pos[e].y) == e);     /* 重ならない */
        }
    }
}

/* ===== 移動・戦闘 ===== */
static void test_move_blocked_by_wall(void) {
    static World w;
    make_flat_world(&w);
    w.pos[w.player] = (CPosition){ 1, 1 };
    CHECK(system_move(&w, w.player, -1, 0) == MOVE_BLOCKED);
    CHECK(system_move(&w, w.player, 0, -1) == MOVE_BLOCKED);
    CHECK(system_move(&w, w.player, 1, 0) == MOVE_MOVED);
    CHECK(w.pos[w.player].x == 2);
}

static void test_bump_attack_and_kill(void) {
    static World w;
    make_flat_world(&w);
    Entity e = add_enemy(&w, 6, 5);   /* プレイヤー(5,5)の右隣 */

    CHECK(system_move(&w, w.player, 1, 0) == MOVE_ATTACKED);
    CHECK(w.pos[w.player].x == 5);                       /* 攻撃したので動かない */
    CHECK(w.health[e].hp == ENEMY_HP - PLAYER_ATK);

    system_move(&w, w.player, 1, 0);                     /* 8 - 4 - 4 = 0 → 撃破 */
    CHECK(!world_has(&w, e, COMP_POSITION));
    CHECK(game_enemies_alive(&w) == 0);
    CHECK(system_move(&w, w.player, 1, 0) == MOVE_MOVED); /* 倒した後は通れる */
}

static void test_enemies_do_not_stack(void) {
    static World w;
    make_flat_world(&w);
    Entity a = add_enemy(&w, 10, 10);
    add_enemy(&w, 11, 10);
    CHECK(system_move(&w, a, 1, 0) == MOVE_BLOCKED);     /* 味方とは重ならず攻撃もしない */
    CHECK(w.health[2].hp == ENEMY_HP);
}

/* ===== 敵AI ===== */
static void test_ai_notices_and_chases(void) {
    static World w;
    make_flat_world(&w);
    Entity e = add_enemy(&w, 5 + ENEMY_VIEW + 3, 5);   /* 索敵範囲外 */
    system_enemy_ai(&w);
    CHECK(w.ai[e].state == AI_IDLE);

    w.pos[e].x = 5 + ENEMY_VIEW;                        /* 範囲内へ */
    system_enemy_ai(&w);
    CHECK(w.ai[e].state == AI_CHASE);
    system_enemy_ai(&w);
    CHECK(w.pos[e].x == 5 + ENEMY_VIEW - 1);            /* 1歩近づく */
}

static void test_enemy_attacks_and_player_dies(void) {
    static World w;
    make_flat_world(&w);
    Entity e = add_enemy(&w, 6, 5);
    w.ai[e].state = AI_CHASE;

    system_enemy_ai(&w);
    CHECK(w.health[w.player].hp == PLAYER_HP - ENEMY_ATK);

    w.health[w.player].hp = 1;
    system_enemy_ai(&w);
    CHECK(w.health[w.player].hp == 0);
    CHECK(w.status == GAME_OVER);
}

/* ===== ターン進行 ===== */
static void test_turn_rules(void) {
    static World w;
    make_flat_world(&w);
    w.pos[w.player] = (CPosition){ 1, 1 };

    CHECK(game_step(&w, CMD_LEFT) == 0);    /* 壁にぶつかってもターンは進まない */
    CHECK(w.turn == 0);
    CHECK(game_step(&w, CMD_SAVE) == 0);
    CHECK(game_step(&w, CMD_RIGHT) == 1);
    CHECK(game_step(&w, CMD_WAIT) == 1);
    CHECK(w.turn == 2);

    w.map.tiles[1][3] = TILE_GOAL;
    game_step(&w, CMD_RIGHT);
    CHECK(w.status == GAME_CLEAR);
    CHECK(game_step(&w, CMD_RIGHT) == 0);   /* 終了後は何もしない */
}

static void test_turn_does_not_overflow(void) {
    static World w;
    make_flat_world(&w);
    w.turn = MAX_TURNS;
    CHECK(game_step(&w, CMD_WAIT) == 1);
    CHECK(w.turn == MAX_TURNS);             /* 上限で止まり、あふれない */
}

static void test_key_mapping(void) {
    CHECK(game_command_from_key('w') == CMD_UP);
    CHECK(game_command_from_key('D') == CMD_RIGHT);
    CHECK(game_command_from_key(KEY_LEFT_ARROW) == CMD_LEFT);
    CHECK(game_command_from_key('q') == CMD_QUIT);
    CHECK(game_command_from_key('z') == CMD_NONE);
}

/* ===== ログ・描画 ===== */
static void test_log_truncates_safely(void) {
    static World w;
    world_init(&w);
    char longmsg[LOG_LEN * 3];
    memset(longmsg, 'x', sizeof longmsg - 1);
    longmsg[sizeof longmsg - 1] = '\0';

    world_log(&w, "%s", longmsg);
    CHECK(strlen(w.log[0]) == LOG_LEN - 1);   /* 切り詰められ、NUL 終端される */
    world_log(&w, "second");
    CHECK(strcmp(w.log[0], "second") == 0);
    CHECK(strlen(w.log[1]) == LOG_LEN - 1);
}

static void test_render(void) {
    static World w;
    static char buf[RENDER_BUF_SIZE];
    game_new(&w, 7);
    size_t n = render_frame(&w, buf, sizeof buf);
    CHECK(n > 0 && n < sizeof buf);
    CHECK(strchr(buf, '@') != NULL);
    CHECK(strstr(buf, "HP 20/20") != NULL);

    char small[32];   /* 小さいバッファでもあふれない */
    n = render_frame(&w, small, sizeof small);
    CHECK(n < sizeof small && small[n] == '\0');   /* 範囲内を確認してから読む */
}

/* ===== セーブ/ロード ===== */
static void test_save_roundtrip(void) {
    static World a, b;
    static uint8_t buf[SAVE_MAX_SIZE];
    game_new(&a, 99);
    game_step(&a, CMD_RIGHT);
    game_step(&a, CMD_DOWN);

    size_t len = save_serialize(&a, buf, sizeof buf);
    CHECK(len > 0);
    CHECK(save_deserialize(buf, len, &b) == SAVE_OK);

    CHECK(b.entity_count == a.entity_count);
    CHECK(b.turn == a.turn && b.seed == a.seed && b.rng.state == a.rng.state);
    CHECK(memcmp(&b.map, &a.map, sizeof a.map) == 0);
    for (Entity e = 0; e < a.entity_count; e++) {
        CHECK(b.mask[e] == a.mask[e]);
        CHECK(b.pos[e].x == a.pos[e].x && b.pos[e].y == a.pos[e].y);
        CHECK(b.health[e].hp == a.health[e].hp);
    }
    /* 同じ状態から同じ操作をすれば同じ結果になる（RNG 状態も保存できている） */
    game_step(&a, CMD_RIGHT);
    game_step(&b, CMD_RIGHT);
    CHECK(a.rng.state == b.rng.state);
}

static void test_save_rejects_bad_data(void) {
    static World src, out, before;
    static uint8_t good[SAVE_MAX_SIZE], buf[SAVE_MAX_SIZE];
    game_new(&src, 5);
    size_t len = save_serialize(&src, good, sizeof good);

    game_new(&out, 1);
    before = out;

    /* マジック不一致 */
    memcpy(buf, good, len); buf[0] = 'X';
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_FORMAT);

    /* バージョン違い */
    memcpy(buf, good, len); buf[4] = 99;
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_VERSION);

    /* 1ビットでも壊れたら CRC で検出 */
    memcpy(buf, good, len); buf[len - 1] ^= 1;
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_CRC);

    /* 途中で切れたファイル */
    CHECK(save_deserialize(good, len - 10, &out) == SAVE_ERR_FORMAT);
    CHECK(save_deserialize(good, 3, &out) == SAVE_ERR_FORMAT);

    /* CRC は正しいが中身が不正なデータ（細工されたファイルを想定） */
    memcpy(buf, good, len); patch_u32(buf, len, OFF_COUNT, 0xFFFFFFFFu);    /* 件数 = -1 */
    CHECK(save_deserialize(buf, len, &out) != SAVE_OK);
    memcpy(buf, good, len); patch_u32(buf, len, OFF_COUNT, 100000u);        /* 件数が巨大 */
    CHECK(save_deserialize(buf, len, &out) != SAVE_OK);
    memcpy(buf, good, len); patch_u32(buf, len, OFF_ENTITY(0, F_X), 9999u); /* 座標が範囲外 */
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_RANGE);
    memcpy(buf, good, len); patch_u32(buf, len, OFF_ENTITY(0, F_HP), 500u); /* HP > 最大HP */
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_RANGE);
    memcpy(buf, good, len); patch_u32(buf, len, OFF_TURN, 2147483647u);     /* ターン数が上限超え（ファザーが発見） */
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_RANGE);
    memcpy(buf, good, len); patch_u32(buf, len, OFF_STATUS, 77u);           /* 未知の状態 */
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_RANGE);
    memcpy(buf, good, len); patch_u32(buf, len, OFF_ENTITY(1, F_AI), 42u);  /* 未知のAI状態 */
    CHECK(save_deserialize(buf, len, &out) == SAVE_ERR_RANGE);

    /* 失敗しても out は書き換わっていない */
    CHECK(memcmp(&out, &before, sizeof out) == 0);
}

static void test_save_file_roundtrip(void) {
    static World a, b;
    const char *path = "test_save.dat";
    game_new(&a, 2024);
    CHECK(save_write_file(&a, path) == SAVE_OK);
    CHECK(save_read_file(&b, path) == SAVE_OK);
    CHECK(b.seed == 2024);

    FILE *fp = fopen("test_save.dat.tmp", "rb");   /* 一時ファイルが残っていない */
    CHECK(fp == NULL);
    if (fp) fclose(fp);

    remove(path);
    CHECK(save_read_file(&b, path) == SAVE_ERR_IO);
}

int main(void) {
    RUN(test_rng_is_deterministic);
    RUN(test_map_always_reachable);
    RUN(test_map_same_seed_same_map);
    RUN(test_entity_limit);
    RUN(test_game_new_spawns_correctly);
    RUN(test_move_blocked_by_wall);
    RUN(test_bump_attack_and_kill);
    RUN(test_enemies_do_not_stack);
    RUN(test_ai_notices_and_chases);
    RUN(test_enemy_attacks_and_player_dies);
    RUN(test_turn_rules);
    RUN(test_turn_does_not_overflow);
    RUN(test_key_mapping);
    RUN(test_log_truncates_safely);
    RUN(test_render);
    RUN(test_save_roundtrip);
    RUN(test_save_rejects_bad_data);
    RUN(test_save_file_roundtrip);

    printf("\n%d checks, %d failed\n", g_checks, g_failed);
    return g_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}