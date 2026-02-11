#include "systems.h"
#include "components.h"
#include <stdlib.h>

void system_attack(Entity attacker, Entity target) {
    if (!has_attack[attacker] || !has_health[target]) return;

    c_health[target].hp -= c_attack[attacker].atk;
}
