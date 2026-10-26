/*
 * ファジングの「種」になる正しいセーブデータを1つ作る。
 * ファザーはこれを少しずつ壊しながら入力を作るので、奥のコードまで届きやすくなる。
 */
#include <stdio.h>

#include "game.h"
#include "save.h"

int main(int argc, char **argv) {
    static World w;
    if (argc < 2) {
        fprintf(stderr, "usage: %s OUTPUT\n", argv[0]);
        return 1;
    }
    game_new(&w, 12345);
    SaveResult r = save_write_file(&w, argv[1]);
    printf("seed corpus: %s (%s)\n", argv[1], save_result_str(r));
    return r == SAVE_OK ? 0 : 1;
}