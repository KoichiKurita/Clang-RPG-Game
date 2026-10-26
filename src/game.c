#include "game.h"

#include <stdlib.h>

#include "platform.h"
#include "systems.h"

#define SPAWN_MIN_DISTANCE 8   /* プレイヤーからこの距離以上離して敵を置く */
#define SPAWN_MAX_TRY    1000

static Entity spawn_player(World *w) {
    Entity e = world_create_entity(w);
    if (e == ENTITY_NONE) return e;
    w->mask[e]   = COMP_POSITION | COMP_HEALTH | COMP_ATTACK | COMP_PLAYER;
    w->pos[e]    = (CPosition){ START_X, START_Y };
    w->health[e] = (CHealth){ PLAYER_HP, PLAYER_HP };
    w->attack[e] = (CAttack){ PLAYER_ATK };
    return e;
}

static int is_spawnable(const World *w, int x, int y) {
    if (w->map.tiles[y][x] != TILE_FLOOR) return 0;              /* 床の上だけ */
    if (world_entity_at(w, x, y) != ENTITY_NONE) return 0;       /* 重ならない */
    int d = abs(x - START_X) + abs(y - START_Y);
    return d >= SPAWN_MIN_DISTANCE;
}

static void spawn_enemy(World *w) {
    for (int i = 0; i < SPAWN_MAX_TRY; i++) {
        int x = rng_range(&w->rng, MAP_WIDTH);
        int y = rng_range(&w->rng, MAP_HEIGHT);
        if (!is_spawnable(w, x, y)) continue;

        Entity e = world_create_entity(w);
        if (e == ENTITY_NONE) return;
        w->mask[e]   = COMP_POSITION | COMP_HEALTH | COMP_ATTACK | COMP_AI | COMP_ENEMY;
        w->pos[e]    = (CPosition){ x, y };
        w->health[e] = (CHealth){ ENEMY_HP, ENEMY_HP };
        w->attack[e] = (CAttack){ ENEMY_ATK };
        w->ai[e]     = (CAI){ AI_IDLE, x, y };
        return;
    }
}

void game_new(World *w, uint32_t seed) {
    world_init(w);
    w->seed = seed;
    rng_seed(&w->rng, seed);
    map_generate(&w->map, &w->rng);

    w->player = spawn_player(w);
    for (int i = 0; i < ENEMY_COUNT; i++) spawn_enemy(w);

    world_log(w, "Reach G to escape the dungeon.");
}

Command game_command_from_key(int key) {
    switch (key) {
    case 'w': case 'W': case KEY_UP_ARROW:    return CMD_UP;
    case 's': case 'S': case KEY_DOWN_ARROW:  return CMD_DOWN;
    case 'a': case 'A': case KEY_LEFT_ARROW:  return CMD_LEFT;
    case 'd': case 'D': case KEY_RIGHT_ARROW: return CMD_RIGHT;
    case ' ': case '.':                       return CMD_WAIT;
    case 'p': case 'P':                       return CMD_SAVE;
    case 'q': case 'Q':                       return CMD_QUIT;
    default:                                  return CMD_NONE;
    }
}

int game_step(World *w, Command cmd) {
    if (w->status != GAME_PLAYING) return 0;

    int dx = 0, dy = 0;
    switch (cmd) {
    case CMD_UP:    dy = -1; break;
    case CMD_DOWN:  dy =  1; break;
    case CMD_LEFT:  dx = -1; break;
    case CMD_RIGHT: dx =  1; break;
    case CMD_WAIT:  break;
    case CMD_QUIT:  w->status = GAME_QUIT; return 0;
    default:        return 0;   /* CMD_NONE / CMD_SAVE はターンを消費しない */
    }

    if (cmd != CMD_WAIT && system_move(w, w->player, dx, dy) == MOVE_BLOCKED) {
        return 0;   /* 壁にぶつかっただけならターンを進めない */
    }

    /* ゴール判定は敵が動く前に行う（ゴールに着いた瞬間にクリア） */
    const CPosition *p = &w->pos[w->player];
    if (w->map.tiles[p->y][p->x] == TILE_GOAL) {
        w->status = GAME_CLEAR;
        w->turn++;
        return 1;
    }

    system_enemy_ai(w);
    w->turn++;
    return 1;
}

int game_enemies_alive(const World *w) {
    int n = 0;
    for (Entity e = 0; e < w->entity_count; e++)
        if (world_has(w, e, COMP_ENEMY)) n++;
    return n;
}