#pragma once

#include "Monster.hpp"
#include "MonsterTraits.hpp"

#include <array>
#include <optional>

class MonsterSelection
{
public:
  MonsterSelection();
  void run();
  Monster get() const;
  [[nodiscard]] std::optional<Monster> toArena();
  [[nodiscard]] std::optional<Monster> toPool();

private:
  MonsterType selectedType;
  int level;
  int dungeonMultiplier;
  int selectedDungeonIndex;
  std::optional<Monster> arenaMonster;
  std::optional<Monster> poolMonster;
};

class CustomMonsterBuilder
{
public:
  CustomMonsterBuilder();
  void run();
  [[nodiscard]] Monster get() const;
  [[nodiscard]] std::optional<Monster> toArena();
  [[nodiscard]] std::optional<Monster> toPool();

private:
  std::array<int, 6> data;
  MonsterTraits traits;
  std::optional<Monster> arenaMonster;
  std::optional<Monster> poolMonster;
};
