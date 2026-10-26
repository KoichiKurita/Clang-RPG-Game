#include "save.h"

#include <stdio.h>
#include <string.h>

#include "platform.h"

#define STAT_LIMIT 9999   /* HP・攻撃力として許容する上限 */

/* ===== CRC32（IEEE 802.3、テーブルなしのビット演算版） ===== */
uint32_t save_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

/* ===== 書き込み側：リトルエンディアンで1バイトずつ並べる =====
 * 構造体を fwrite するとパディングやエンディアンが環境依存になるため避ける */
typedef struct { uint8_t *p; size_t cap, len; int overflow; } Writer;

static void put_u32(Writer *w, uint32_t v) {
    if (w->len + 4 > w->cap) { w->overflow = 1; return; }
    for (int i = 0; i < 4; i++) w->p[w->len++] = (uint8_t)(v >> (8 * i));
}
static void put_i32(Writer *w, int32_t v) { put_u32(w, (uint32_t)v); }
static void put_bytes(Writer *w, const void *src, size_t n) {
    if (w->len + n > w->cap) { w->overflow = 1; return; }
    memcpy(w->p + w->len, src, n);
    w->len += n;
}

/* ===== 読み込み側：範囲外を読もうとしたら失敗フラグを立てる ===== */
typedef struct { const uint8_t *p; size_t len, pos; int error; } Reader;

static uint32_t get_u32(Reader *r) {
    if (r->len - r->pos < 4) { r->error = 1; return 0; }
    uint32_t v = 0;
    for (int i = 0; i < 4; i++) v |= (uint32_t)r->p[r->pos++] << (8 * i);
    return v;
}
static int32_t get_i32(Reader *r) { return (int32_t)get_u32(r); }
static void get_bytes(Reader *r, void *dst, size_t n) {
    if (r->len - r->pos < n) { r->error = 1; return; }
    memcpy(dst, r->p + r->pos, n);
    r->pos += n;
}

size_t save_serialize(const World *w, uint8_t *buf, size_t cap) {
    Writer wr = { buf, cap, 0, 0 };
    uint32_t payload_len = SAVE_FIXED_SIZE + SAVE_ENTITY_SIZE * (uint32_t)w->entity_count;

    /* ヘッダ（CRC は最後に埋める） */
    put_bytes(&wr, SAVE_MAGIC, 4);
    put_u32(&wr, SAVE_VERSION);
    put_u32(&wr, payload_len);
    put_u32(&wr, 0);

    put_u32(&wr, w->rng.state);
    put_u32(&wr, w->seed);
    put_i32(&wr, w->turn);
    put_i32(&wr, (int32_t)w->status);
    put_i32(&wr, w->player);
    put_i32(&wr, w->entity_count);
    put_bytes(&wr, w->map.tiles, sizeof w->map.tiles);

    for (Entity e = 0; e < w->entity_count; e++) {
        put_u32(&wr, w->mask[e]);
        put_i32(&wr, w->pos[e].x);
        put_i32(&wr, w->pos[e].y);
        put_i32(&wr, w->health[e].hp);
        put_i32(&wr, w->health[e].max_hp);
        put_i32(&wr, w->attack[e].atk);
        put_i32(&wr, (int32_t)w->ai[e].state);
        put_i32(&wr, w->ai[e].home_x);
        put_i32(&wr, w->ai[e].home_y);
    }
    if (wr.overflow) return 0;

    uint32_t crc = save_crc32(buf + SAVE_HEADER_SIZE, wr.len - SAVE_HEADER_SIZE);
    for (int i = 0; i < 4; i++) buf[12 + i] = (uint8_t)(crc >> (8 * i));
    return wr.len;
}

/* 復元した World の中身が「ありえる値」かを検証する */
static SaveResult validate(const World *w) {
    if (w->status < 0 || w->status >= GAME_STATUS_COUNT) return SAVE_ERR_RANGE;
    // if (w->turn < 0) return SAVE_ERR_RANGE;
    if (w->turn < 0 || w->turn > MAX_TURNS) return SAVE_ERR_RANGE;

    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            if (!map_is_valid_tile(w->map.tiles[y][x])) return SAVE_ERR_RANGE;

    if (!world_is_valid(w, w->player)) return SAVE_ERR_RANGE;
    if (!world_has(w, w->player, COMP_PLAYER | COMP_POSITION | COMP_HEALTH | COMP_ATTACK))
        return SAVE_ERR_RANGE;

    for (Entity e = 0; e < w->entity_count; e++) {
        uint32_t m = w->mask[e];
        if (m & ~(uint32_t)COMP_ALL) return SAVE_ERR_RANGE;                /* 未知のビット */
        if ((m & COMP_PLAYER) && e != w->player) return SAVE_ERR_RANGE;   /* プレイヤーは1人 */
        if ((m & COMP_PLAYER) && (m & COMP_ENEMY)) return SAVE_ERR_RANGE;

        if ((m & COMP_POSITION) && !map_is_walkable(&w->map, w->pos[e].x, w->pos[e].y))
            return SAVE_ERR_RANGE;
        if (m & COMP_HEALTH) {
            const CHealth *h = &w->health[e];
            if (h->max_hp <= 0 || h->max_hp > STAT_LIMIT) return SAVE_ERR_RANGE;
            if (h->hp < 0 || h->hp > h->max_hp) return SAVE_ERR_RANGE;
        }
        if ((m & COMP_ATTACK) && (w->attack[e].atk < 0 || w->attack[e].atk > STAT_LIMIT))
            return SAVE_ERR_RANGE;
        if (m & COMP_AI) {
            const CAI *ai = &w->ai[e];
            if (ai->state < 0 || ai->state >= AI_STATE_COUNT) return SAVE_ERR_RANGE;
            if (!map_in_bounds(ai->home_x, ai->home_y)) return SAVE_ERR_RANGE;
        }
    }
    return SAVE_OK;
}

SaveResult save_deserialize(const uint8_t *buf, size_t len, World *out) {
    if (len < SAVE_HEADER_SIZE + SAVE_FIXED_SIZE) return SAVE_ERR_FORMAT;
    if (memcmp(buf, SAVE_MAGIC, 4) != 0) return SAVE_ERR_FORMAT;

    Reader r = { buf, len, 4, 0 };
    uint32_t version     = get_u32(&r);
    uint32_t payload_len = get_u32(&r);
    uint32_t crc         = get_u32(&r);

    if (version != SAVE_VERSION) return SAVE_ERR_VERSION;
    if (payload_len != len - SAVE_HEADER_SIZE) return SAVE_ERR_FORMAT;
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    /* ファジング時だけ CRC 検査を外す（ランダム入力が CRC で全部弾かれ、奥まで届かないため） */
    if (save_crc32(buf + SAVE_HEADER_SIZE, payload_len) != crc) return SAVE_ERR_CRC;
#else
    (void)crc;
#endif

    /* いきなり *out に書かず、一時領域で組み立てて検証してからコピーする */
    World tmp;
    world_init(&tmp);

    tmp.rng.state    = get_u32(&r);
    tmp.seed         = get_u32(&r);
    tmp.turn         = get_i32(&r);
    tmp.status       = (GameStatus)get_i32(&r);
    tmp.player       = get_i32(&r);
    int32_t count    = get_i32(&r);

    /* 件数は「使う前に」検証する。負数や巨大値で配列外アクセスさせない */
    if (count < 1 || count > MAX_ENTITIES) return SAVE_ERR_RANGE;
    if (payload_len != SAVE_FIXED_SIZE + SAVE_ENTITY_SIZE * (uint32_t)count) return SAVE_ERR_FORMAT;
    tmp.entity_count = count;

    get_bytes(&r, tmp.map.tiles, sizeof tmp.map.tiles);

    for (Entity e = 0; e < count; e++) {
        tmp.mask[e]          = get_u32(&r);
        tmp.pos[e].x         = get_i32(&r);
        tmp.pos[e].y         = get_i32(&r);
        tmp.health[e].hp     = get_i32(&r);
        tmp.health[e].max_hp = get_i32(&r);
        tmp.attack[e].atk    = get_i32(&r);
        tmp.ai[e].state      = (AIState)get_i32(&r);
        tmp.ai[e].home_x     = get_i32(&r);
        tmp.ai[e].home_y     = get_i32(&r);
    }
    if (r.error) return SAVE_ERR_FORMAT;

    SaveResult v = validate(&tmp);
    if (v != SAVE_OK) return v;

    *out = tmp;   /* 全部の検証に通った時だけ反映 */
    return SAVE_OK;
}

SaveResult save_write_file(const World *w, const char *path) {
    static uint8_t buf[SAVE_MAX_SIZE];
    size_t len = save_serialize(w, buf, sizeof buf);
    if (len == 0) return SAVE_ERR_FORMAT;

    char tmp_path[260];
    int n = snprintf(tmp_path, sizeof tmp_path, "%s.tmp", path);
    if (n < 0 || (size_t)n >= sizeof tmp_path) return SAVE_ERR_IO;

    FILE *fp = fopen(tmp_path, "wb");
    if (!fp) return SAVE_ERR_IO;
    int ok = fwrite(buf, 1, len, fp) == len;
    ok = (fflush(fp) == 0) && ok;
    ok = (fclose(fp) == 0) && ok;
    if (!ok) { remove(tmp_path); return SAVE_ERR_IO; }

    /* 書き込みが完全に終わってから本物と入れ替える。
     * 途中で落ちても元の save.dat は壊れない */
    if (platform_replace_file(tmp_path, path) != 0) { remove(tmp_path); return SAVE_ERR_IO; }
    return SAVE_OK;
}

SaveResult save_read_file(World *out, const char *path) {
    static uint8_t buf[SAVE_MAX_SIZE + 1];
    FILE *fp = fopen(path, "rb");
    if (!fp) return SAVE_ERR_IO;

    size_t len = fread(buf, 1, sizeof buf, fp);
    int err = ferror(fp);
    fclose(fp);

    if (err) return SAVE_ERR_IO;
    if (len > SAVE_MAX_SIZE) return SAVE_ERR_FORMAT;   /* 想定より大きいファイルは拒否 */
    return save_deserialize(buf, len, out);
}

const char *save_result_str(SaveResult r) {
    switch (r) {
    case SAVE_OK:          return "OK";
    case SAVE_ERR_IO:      return "I/O error";
    case SAVE_ERR_FORMAT:  return "invalid format";
    case SAVE_ERR_VERSION: return "unsupported version";
    case SAVE_ERR_CRC:     return "checksum mismatch (file corrupted)";
    case SAVE_ERR_RANGE:   return "value out of range";
    default:               return "unknown error";
    }
}