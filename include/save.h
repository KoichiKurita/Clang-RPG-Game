#ifndef SAVE_H
#define SAVE_H

#include <stddef.h>
#include <stdint.h>

#include "world.h"

/*
 * セーブファイル形式（すべてリトルエンディアン）
 *
 *   [Header 16 bytes]
 *     magic       4 bytes  "CRPG"
 *     version     u32      SAVE_VERSION
 *     payload_len u32      Payload のバイト数
 *     crc32       u32      Payload の CRC32（破損検知用。改ざん防止ではない）
 *   [Payload]
 *     rng_state u32, seed u32, turn i32, status i32, player i32, entity_count i32
 *     tiles     MAP_WIDTH * MAP_HEIGHT bytes
 *     entity × entity_count:
 *       mask u32, x, y, hp, max_hp, atk, ai_state, home_x, home_y (各 i32)
 */

#define SAVE_MAGIC         "CRPG"
#define SAVE_VERSION       1u
#define SAVE_HEADER_SIZE   16u
#define SAVE_FIXED_SIZE    (24u + (uint32_t)(MAP_WIDTH * MAP_HEIGHT))
#define SAVE_ENTITY_SIZE   36u
#define SAVE_MAX_SIZE      (SAVE_HEADER_SIZE + SAVE_FIXED_SIZE + SAVE_ENTITY_SIZE * MAX_ENTITIES)

typedef enum {
    SAVE_OK = 0,
    SAVE_ERR_IO,         /* ファイルが開けない・書けない */
    SAVE_ERR_FORMAT,     /* マジック不一致・サイズ不正 */
    SAVE_ERR_VERSION,    /* 未対応バージョン */
    SAVE_ERR_CRC,        /* データ破損 */
    SAVE_ERR_RANGE       /* 値が範囲外（座標・HP・状態など） */
} SaveResult;

/* World をバイト列に変換する。書き込んだバイト数を返す（容量不足なら 0） */
size_t     save_serialize(const World *w, uint8_t *buf, size_t cap);

/* バイト列を検証しながら World に復元する。
 * 失敗した場合 *out は一切変更しない。 */
SaveResult save_deserialize(const uint8_t *buf, size_t len, World *out);

/* ファイルへの保存（一時ファイル → 置き換え でアトミックに保存） */
SaveResult save_write_file(const World *w, const char *path);
SaveResult save_read_file(World *out, const char *path);

const char *save_result_str(SaveResult r);
uint32_t    save_crc32(const uint8_t *data, size_t len);

#endif
