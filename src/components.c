#include "components.h"

CPosition c_position[MAX_ENTITIES];
CHealth   c_health[MAX_ENTITIES];
CAttack   c_attack[MAX_ENTITIES];
CTag      c_tag[MAX_ENTITIES];

int has_position[MAX_ENTITIES] = {0};
int has_health[MAX_ENTITIES]   = {0};
int has_attack[MAX_ENTITIES]   = {0};
int has_tag[MAX_ENTITIES]      = {0};
