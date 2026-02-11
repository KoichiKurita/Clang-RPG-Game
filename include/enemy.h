#ifndef ENEMY_H
#define ENEMY_H

typedef enum {
    ENEMY_IDLE,
    ENEMY_CHASE,
    ENEMY_RETURN
} EnemyState;

typedef struct {
    int x, y;
    int ox, oy;
    int hp;
    EnemyState state;
    int alive;
} Enemy;

void init_enemy(Enemy* e, int x, int y);
void update_enemy(Enemy* e, int px, int py, char map[][40]);

#endif
