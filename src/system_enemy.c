#include "systems.h"
#include "components.h"

/*
 * 敵AI System
 * ・生存している敵のみ処理
 * ・プレイヤーを追跡
 * ・壁には入らない
 */
void system_enemy_ai(
    Entity enemy,
    Entity player,
    char map[MAP_HEIGHT][MAP_WIDTH]
) {
    // 必須コンポーネントチェック
    if (!has_position[enemy] || !has_position[player]) return;
    if (!has_health[enemy] || c_health[enemy].hp <= 0) return;

    int dx = 0;
    int dy = 0;

    // プレイヤー方向へ1マス近づく
    if (c_position[player].x < c_position[enemy].x) dx = -1;
    else if (c_position[player].x > c_position[enemy].x) dx = 1;

    if (c_position[player].y < c_position[enemy].y) dy = -1;
    else if (c_position[player].y > c_position[enemy].y) dy = 1;

    // 移動（既存の移動Systemを再利用）
    system_movement(enemy, dx, dy, map);
}
