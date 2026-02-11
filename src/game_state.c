#include "game_state.h"

void init_game(GameState* g) {
    g->status = GAME_PLAYING;
    g->turn = 0;
}
