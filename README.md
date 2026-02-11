# Console RPG (C / ECS)

C言語で作成した **コンソール型ローグライクRPG** です。  
ECS（Entity Component System）設計を採用し、拡張性・保守性を重視した構成になっています。

---

## 🎮 ゲーム概要

- ランダム生成されるダンジョンを探索
- 敵を避ける／倒しながらゴールを目指す
- ゴール (`G`) に到達するとゲームクリア

---

## ✨ 特徴

- C言語による本格的なゲーム設計
- ECS（Entity / Component / System）構造
- 必ずゴールに到達可能なランダムマップ生成
- 複数敵エンティティ対応
- 敵AI（索敵・追跡）
- 戦闘ログ表示
- オートセーブ（拡張対応）

---

## 🖥 動作環境

- OS: Windows 10 / 11
- Compiler: GCC (MinGW / MSYS2)
- Language: C (C99)

---

## 🔧 ビルド方法

### コンパイル

```bash
gcc src/*.c -Iinclude -Wall -Wextra -std=c99 -o rpg_game
