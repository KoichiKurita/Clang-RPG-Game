/*
 * libFuzzer ハーネス：セーブデータの読み込み処理にランダムなバイト列を与え続け、
 * クラッシュ・メモリ破壊・未定義動作が起きないことを確かめる。
 * 実行: make fuzz（clang が必要）
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "game.h"
#include "render.h"
#include "save.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    static World w;
    static char frame[RENDER_BUF_SIZE];

    if (save_deserialize(data, size, &w) != SAVE_OK) return 0;

    /* ロードに成功したデータは「安全に遊べる」はず。実際に動かして確かめる */
    render_frame(&w, frame, sizeof frame);
    for (int i = 0; i < 8 && w.status == GAME_PLAYING; i++) {
        game_step(&w, (Command)(CMD_UP + (i % 5)));
    }
    return 0;
}
