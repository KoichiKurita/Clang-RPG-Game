#include "save.h"
#include <stdio.h>

/*
  戻り値:
   1 = 成功
   0 = 失敗
*/

int save_game(
    Player* player,
    Enemy enemies[],
    int enemy_count,
    char map[MAP_HEIGHT][MAP_WIDTH],
    GameState* game
) {
    FILE* fp = fopen(SAVE_FILE, "wb");
    if (!fp) return 0;

    fwrite(player, sizeof(Player), 1, fp);
    fwrite(&enemy_count, sizeof(int), 1, fp);
    fwrite(enemies, sizeof(Enemy), enemy_count, fp);
    fwrite(map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, fp);
    fwrite(game, sizeof(GameState), 1, fp);

    fclose(fp);
    return 1;
}

int load_game(
    Player* player,
    Enemy enemies[],
    int enemy_count,
    char map[MAP_HEIGHT][MAP_WIDTH],
    GameState* game
) {
    FILE* fp = fopen(SAVE_FILE, "rb");
    if (!fp) return 0;

    int saved_enemy_count = 0;

    fread(player, sizeof(Player), 1, fp);
    fread(&saved_enemy_count, sizeof(int), 1, fp);

    /* セーブ時と敵数が違っても壊れないようにする */
    int read_count =
        (saved_enemy_count < enemy_count)
        ? saved_enemy_count
        : enemy_count;

    fread(enemies, sizeof(Enemy), read_count, fp);

    /* 余った敵データは読み飛ばす */
    if (saved_enemy_count > enemy_count) {
        fseek(fp,
              sizeof(Enemy) * (saved_enemy_count - enemy_count),
              SEEK_CUR);
    }

    fread(map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, fp);
    fread(game, sizeof(GameState), 1, fp);

    fclose(fp);
    return 1;
}
