#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "platform.h"
#include "render.h"
#include "save.h"

static World g_world;   /* ゲーム全体の状態（この1つだけ） */

static void print_usage(const char *prog) {
    printf("Usage: %s [--seed N] [--new]\n", prog);
    printf("  --seed N   generate the dungeon from seed N (reproducible)\n");
    printf("  --new      ignore the save file and start a new game\n");
}

/* 引数を解析する。不正な引数なら 0 を返す */
static int parse_args(int argc, char **argv, uint32_t *seed, int *has_seed, int *force_new) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            char *end = NULL;
            unsigned long v = strtoul(argv[++i], &end, 10);
            if (end == argv[i] || *end != '\0' || v > UINT32_MAX) return 0;   /* 数値以外は拒否 */
            *seed = (uint32_t)v;
            *has_seed = 1;
        } else if (strcmp(argv[i], "--new") == 0) {
            *force_new = 1;
        } else {
            return 0;
        }
    }
    return 1;
}

static void draw(const World *w) {
    static char frame[RENDER_BUF_SIZE];
    size_t n = render_frame(w, frame, sizeof frame);
    platform_write(frame, n);
}

static void save_with_log(World *w) {
    SaveResult r = save_write_file(w, SAVE_FILE);
    if (r == SAVE_OK) world_log(w, "Game saved.");
    else              world_log(w, "Save failed: %s", save_result_str(r));
}

/* Q は W・A の隣にあり誤って押しやすいので、終了前に確認する。終了するなら 1 */
static int confirm_quit(World *w) {
    world_log(w, "Save & quit? [Y] yes / any other key: continue");
    draw(w);
    int k = platform_read_key();
    if (k == 'y' || k == 'Y') return 1;
    world_log(w, "Continue.");
    return 0;
}

/* セーブがあれば続きから遊ぶか聞く。ロードしたら 1 */
static int try_continue(World *w) {
    World loaded;
    SaveResult r = save_read_file(&loaded, SAVE_FILE);
    if (r == SAVE_ERR_IO) return 0;   /* セーブなし */
    if (r != SAVE_OK) {
        printf("Save file is broken (%s). Starting a new game.\n", save_result_str(r));
        printf("Press any key...\n");
        platform_read_key();
        return 0;
    }
    if (loaded.status != GAME_PLAYING) return 0;

    printf("Save found (turn %d). [C]ontinue / [N]ew game\n", loaded.turn);
    for (;;) {
        int k = platform_read_key();
        if (k == 'c' || k == 'C') {
            *w = loaded;
            world_log(w, "Welcome back!");
            return 1;
        }
        if (k == 'n' || k == 'N') return 0;
    }
}

int main(int argc, char **argv) {
    uint32_t seed = 0;
    int has_seed = 0, force_new = 0;
    if (!parse_args(argc, argv, &seed, &has_seed, &force_new)) {
        print_usage(argv[0]);
        return 1;
    }
    if (!has_seed) seed = (uint32_t)time(NULL);

    platform_init();
    platform_clear();

    World *w = &g_world;
    if (force_new || has_seed || !try_continue(w)) {
        game_new(w, seed);
    }
    platform_clear();

    while (w->status == GAME_PLAYING) {
        draw(w);

        Command cmd = game_command_from_key(platform_read_key());
        if (cmd == CMD_SAVE) { save_with_log(w); continue; }
        if (cmd == CMD_QUIT && !confirm_quit(w)) continue;

        int advanced = game_step(w, cmd);
        if (advanced && w->status == GAME_PLAYING && w->turn % AUTOSAVE_INTERVAL == 0) {
            save_with_log(w);
        }
    }

    if (w->status == GAME_QUIT) {
        w->status = GAME_PLAYING;   /* 続きから遊べる状態で保存する */
        SaveResult r = save_write_file(w, SAVE_FILE);
        platform_clear();
        printf("Saved (%s). See you!\n", save_result_str(r));
        return 0;
    }

    draw(w);
    remove(SAVE_FILE);   /* 決着がついたらセーブは消す */
    printf("\n%s  (turn %d, seed %lu)\n",
           w->status == GAME_CLEAR ? "*** GAME CLEAR ***" : "*** GAME OVER ***",
           w->turn, (unsigned long)w->seed);
    return 0;
}
