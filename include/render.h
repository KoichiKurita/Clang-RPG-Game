#ifndef RENDER_H
#define RENDER_H

#include <stddef.h>

#include "world.h"

#define RENDER_BUF_SIZE 16384

/*
 * 1フレーム分の文字列を buf に組み立て、書き込んだバイト数を返す。
 * 画面へは書かない（出力は呼び出し側が platform_write で1回だけ行う）。
 * 画面と切り離しているので、テストで描画結果を検証できる。
 */
size_t render_frame(const World *w, char *buf, size_t cap);

#endif
