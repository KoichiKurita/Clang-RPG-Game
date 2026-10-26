#ifndef CONFIG_H
#define CONFIG_H

/* ===== マップ ===== */
#define MAP_WIDTH   40
#define MAP_HEIGHT  20

/* ===== エンティティ ===== */
#define MAX_ENTITIES 64
#define ENEMY_COUNT   5

/* ===== ゲームバランス ===== */
#define PLAYER_HP     20
#define PLAYER_ATK     4
#define ENEMY_HP       8
#define ENEMY_ATK      2
#define ENEMY_VIEW     5   /* 索敵半径（この距離以内で追跡開始） */
#define ENEMY_LOSE     8   /* この距離を超えたら追跡をあきらめる */

/* ===== ターン ===== */
#define MAX_TURNS  999999   /* ターン数の上限（int のオーバーフロー防止） */

/* ===== セーブ ===== */
#define SAVE_FILE          "save.dat"
#define AUTOSAVE_INTERVAL  10   /* 何ターンごとにオートセーブするか */

/* ===== ログ ===== */
#define LOG_LINES  5
#define LOG_LEN   64

#endif