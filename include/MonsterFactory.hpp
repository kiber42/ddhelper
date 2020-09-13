#pragma once

#include "Monster.hpp"
#include "MonsterStats.hpp"

enum class MonsterType
{
  Bandit,
  DragonSpawn,
  Goat,
  Goblin,
  Golem,
  GooBlob,
  Gorgon,
  MeatMan,
  Serpent,
  Warlock,
  Wraith,
  Zombie,
};

constexpr const char* toString(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Bandit:
    return "Bandit";
  case MonsterType::DragonSpawn:
    return "Dragon Spawn";
  case MonsterType::Goat:
    return "Goat";
  case MonsterType::Goblin:
    return "Goblin";
  case MonsterType::Golem:
    return "Golem";
  case MonsterType::GooBlob:
    return "Goo Blob";
  case MonsterType::Gorgon:
    return "Gorgon";
  case MonsterType::MeatMan:
    return "Meat Man";
  case MonsterType::Serpent:
    return "Serpent";
  case MonsterType::Warlock:
    return "Warlock";
  case MonsterType::Wraith:
    return "Wraith";
  case MonsterType::Zombie:
    return "Zombie";
  }
}

Monster makeMonster(MonsterType type, int level);

Monster makeGenericMonster(int level, int hp, int damage);
MonsterStats makeGenericMonsterStats(int level, int hp, int damage, int deathProtection = 0);
