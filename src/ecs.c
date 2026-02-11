#include "ecs.h"

static int next_entity = 0;

Entity ecs_create(void) {
    return next_entity++;
}
