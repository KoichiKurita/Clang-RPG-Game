#include "log.h"
#include <stdio.h>
#include <string.h>

static char logbuf[5][64];

void log_push(const char* msg) {
    for (int i = 4; i > 0; i--)
        strcpy(logbuf[i], logbuf[i - 1]);
    strncpy(logbuf[0], msg, 63);
}

void log_draw(void) {
    printf("\n--- LOG ---\n");
    for (int i = 4; i >= 0; i--)
        if (logbuf[i][0]) printf("%s\n", logbuf[i]);
}
