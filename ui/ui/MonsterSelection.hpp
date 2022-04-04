#pragma once

#include "engine/DungeonInfo.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTraits.hpp"

#include <array>
#include <optional>

namespace ui
{
  class MonsterSelection
  {
  public:
    MonsterSelection();
    void run();
    Monster get() const;
    [[nodiscard]] std::optional<Monster> toArena();
    [[nodiscard]] std::optional<Monster> toPool();

    static void runDungeonSelection(Dungeon& selected);

  private:
    MonsterType selectedType;
    Level level;
    Dungeon selectedDungeon;
    std::optional<Monster> arenaMonster;
    std::optional<Monster> poolMonster;
  };

  class CustomMonsterBuilder : public MonsterTraits
  {
  public:
    CustomMonsterBuilder();
    void run();
    [[nodiscard]] Monster get() const;
    [[nodiscard]] std::optional<Monster> toArena();
    [[nodiscard]] std::optional<Monster> toPool();

  private:
    std::array<int, 7> data;
    std::optional<Monster> arenaMonster;
    std::optional<Monster> poolMonster;
  };
} // namespace ui
