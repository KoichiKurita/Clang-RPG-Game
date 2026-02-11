#include "combat.h"
#include <stdlib.h>

void player_attack(Player* p, Enemy* e) {
    if (!e->alive) return;
    if (abs(p->x - e->x) + abs(p->y - e->y) == 1) {
        e->hp -= p->atk;
        if (e->hp <= 0) e->alive = 0;
    }
}
