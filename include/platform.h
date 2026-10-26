#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>

/*
 * OS依存処理をこのモジュールに閉じ込める。
 * 実装: src/platform_win.c（Windows） / src/platform_posix.c（Linux・macOS）
 * どちらも #ifdef で囲まれているため、src 配下の .c をまとめてビルドしてよい。
 */

/* 特殊キー（ASCIIと重ならない値） */
enum {
    KEY_UP_ARROW = 1000,
    KEY_DOWN_ARROW,
    KEY_LEFT_ARROW,
    KEY_RIGHT_ARROW
};

/* コンソールを初期化する（VTモード有効化・UTF-8・カーソル非表示）。
 * 終了時の復元は内部で atexit に登録される。 */
void platform_init(void);

/* キーを1つ読む（入力があるまで待つ）。ASCII または KEY_* を返す。 */
int platform_read_key(void);

/* 文字列をまとめて出力する */
void platform_write(const char *buf, size_t len);

/* 画面を消去してカーソルを左上へ */
void platform_clear(void);

/* tmp を dst へ置き換える（dst が存在しても上書き）。成功で 0。 */
int platform_replace_file(const char *tmp, const char *dst);

#endif
