#include "engine/DungeonInfo.hpp"

std::vector<Monster> boss(Dungeon dungeon)
{
  switch (dungeon)
  {
  case Dungeon::MonsterMachine1:
    return {Boss::create(BossType::Chzar)};
  default:
    return {};
  }
}

std::vector<BossType> boss(Dungeon dungeon, MonsterType type)
{
  switch (dungeon)
  {
  case Dungeon::MonsterMachine1:
    if (type == MonsterType::Ratling)
      return {BossType::Chzar};
  default:
    break;
  }
  return {};
}
