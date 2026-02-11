#ifndef GAME_STATE_H
#define GAME_STATE_H

typedef enum {
    GAME_PLAYING,
    GAME_CLEAR,
    GAME_OVER
} GameStatus;

typedef struct {
    GameStatus status;
    int turn;
} GameState;

void init_game(GameState* g);

#endif
