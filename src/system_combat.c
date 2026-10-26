#include "systems.h"

static const char *name_of(const World *w, Entity e) {
    return world_has(w, e, COMP_PLAYER) ? "You" : "Enemy";
}

void system_attack(World *w, Entity attacker, Entity target) {
    if (!world_has(w, attacker, COMP_ATTACK)) return;
    if (!world_has(w, target, COMP_HEALTH))   return;

    int dmg = w->attack[attacker].atk;
    w->health[target].hp -= dmg;
    world_log(w, "%s hit %s for %d", name_of(w, attacker), name_of(w, target), dmg);

    if (w->health[target].hp > 0) return;

    w->health[target].hp = 0;
    if (target == w->player) {
        world_log(w, "You died...");
        w->status = GAME_OVER;
    } else {
        world_log(w, "Enemy defeated!");
        world_destroy_entity(w, target);
    }
}