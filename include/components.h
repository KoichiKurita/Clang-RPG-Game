#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "ecs.h"

typedef struct {
    int x, y;
} CPosition;

typedef struct {
    int hp, max_hp;
} CHealth;

typedef struct {
    int atk;
} CAttack;

typedef struct {
    int is_player;
} CTag;

extern CPosition c_position[MAX_ENTITIES];
extern CHealth   c_health[MAX_ENTITIES];
extern CAttack   c_attack[MAX_ENTITIES];
extern CTag      c_tag[MAX_ENTITIES];

extern int has_position[MAX_ENTITIES];
extern int has_health[MAX_ENTITIES];
extern int has_attack[MAX_ENTITIES];
extern int has_tag[MAX_ENTITIES];

#endif
