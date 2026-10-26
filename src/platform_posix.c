#ifndef _WIN32

#define _POSIX_C_SOURCE 200809L   /* -std=c99 で termios/sigaction を使うため */

#include "platform.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios g_orig_termios;
static int g_initialized = 0;

static void restore_console(void) {
    if (!g_initialized) return;
    static const char seq[] = "\x1b[0m\x1b[?25h";
    (void)!write(STDOUT_FILENO, seq, sizeof seq - 1);   /* シグナル安全な write を使う */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
    g_initialized = 0;
}

static void on_signal(int sig) {
    restore_console();
    signal(sig, SIG_DFL);
    raise(sig);   /* 既定動作で終了させる */
}

void platform_init(void) {
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) == 0) {
        struct termios raw = g_orig_termios;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);   /* 1文字ずつ・エコーなし */
        raw.c_cc[VMIN]  = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        g_initialized = 1;
    }
    atexit(restore_console);

    struct sigaction sa;
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("\x1b[?25l");
    fflush(stdout);
}

int platform_read_key(void) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return 'q';   /* EOF は終了扱い */

    /* 矢印キーは ESC [ A〜D の3バイト */
    if (c == 0x1b) {
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return 0x1b;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return 0x1b;
        if (seq[0] == '[') {
            switch (seq[1]) {
            case 'A': return KEY_UP_ARROW;
            case 'B': return KEY_DOWN_ARROW;
            case 'C': return KEY_RIGHT_ARROW;
            case 'D': return KEY_LEFT_ARROW;
            default:  return 0;
            }
        }
        return 0;
    }
    return c;
}

void platform_write(const char *buf, size_t len) {
    fwrite(buf, 1, len, stdout);
    fflush(stdout);
}

void platform_clear(void) {
    static const char seq[] = "\x1b[2J\x1b[H";
    platform_write(seq, sizeof seq - 1);
}

int platform_replace_file(const char *tmp, const char *dst) {
    return rename(tmp, dst);   /* POSIX の rename は上書きかつアトミック */
}

#else
typedef int platform_posix_unused;
#endif