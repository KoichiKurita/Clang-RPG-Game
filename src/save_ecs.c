#include <stdio.h>
#include "components.h"
#include "map.h"

void autosave(char map[MAP_HEIGHT][MAP_WIDTH]) {
    FILE* fp = fopen("autosave.dat","wb");
    if (!fp) return;

    fwrite(c_position, sizeof(CPosition), MAX_ENTITIES, fp);
    fwrite(c_health,   sizeof(CHealth),   MAX_ENTITIES, fp);
    fwrite(c_attack,   sizeof(CAttack),   MAX_ENTITIES, fp);
    fwrite(has_position, sizeof(int), MAX_ENTITIES, fp);
    fwrite(has_health,   sizeof(int), MAX_ENTITIES, fp);
    fwrite(map, sizeof(char), MAP_HEIGHT*MAP_WIDTH, fp);

    fclose(fp);
}
