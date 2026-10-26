#ifdef _WIN32

#include "platform.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

static DWORD g_orig_mode;
static UINT  g_orig_cp;
static int   g_initialized = 0;

static void restore_console(void) {
    if (!g_initialized) return;
    printf("\x1b[0m\x1b[?25h");   /* 色リセット・カーソル再表示 */
    fflush(stdout);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), g_orig_mode);
    SetConsoleOutputCP(g_orig_cp);
    g_initialized = 0;
}

/* Ctrl+C / ウィンドウを閉じた時もコンソール設定を戻す */
static BOOL WINAPI ctrl_handler(DWORD type) {
    (void)type;
    restore_console();
    return FALSE;   /* 既定の処理（プロセス終了）に任せる */
}

void platform_init(void) {
    /* MSYS2 の mintty などから起動すると、標準入力が Windows コンソールではなくパイプになる。
     * その場合 _getch はキー入力を受け取れず固まるので、案内を出して終了する。 */
    DWORD in_mode;
    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &in_mode)) {
        fprintf(stderr,
            "This game needs a Windows console.\n"
            "Run it from Windows Terminal (PowerShell / cmd):  .\\rpg.exe\n"
            "From MSYS2 (mintty), use:  winpty ./rpg.exe\n");
        exit(EXIT_FAILURE);
    }

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(out, &g_orig_mode);
    g_orig_cp = GetConsoleOutputCP();

    SetConsoleMode(out, g_orig_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
    g_initialized = 1;

    atexit(restore_console);
    SetConsoleCtrlHandler(ctrl_handler, TRUE);

    printf("\x1b[?25l");   /* カーソル非表示 */
    fflush(stdout);
}

int platform_read_key(void) {
    int c = _getch();
    /* 矢印キーは 0 または 224 の後にもう1バイト来る */
    if (c == 0 || c == 224) {
        switch (_getch()) {
        case 72: return KEY_UP_ARROW;
        case 80: return KEY_DOWN_ARROW;
        case 75: return KEY_LEFT_ARROW;
        case 77: return KEY_RIGHT_ARROW;
        default: return 0;
        }
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
    /* Windows の rename は上書きできないため MoveFileEx を使う */
    return MoveFileExA(tmp, dst, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)
           ? 0 : -1;
}

#else
/* ISO C は空の翻訳単位を禁止しているため、ダミー宣言を置く */
typedef int platform_win_unused;
#endif
