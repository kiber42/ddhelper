#pragma once

#include "MonsterTraits.hpp"

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

constexpr int getHPMultiplierPercent(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Bandit:
    return 100;
  case MonsterType::DragonSpawn:
    return 125;
  case MonsterType::Goat:
    return 90;
  case MonsterType::Goblin:
    return 100;
  case MonsterType::Golem:
    return 100;
  case MonsterType::GooBlob:
    return 100;
  case MonsterType::Gorgon:
    return 90;
  case MonsterType::MeatMan:
    return 200;
  case MonsterType::Serpent:
    return 100;
  case MonsterType::Warlock:
    return 100;
  case MonsterType::Wraith:
    return 75;
  case MonsterType::Zombie:
    return 150;
  }
}

constexpr int getDamageMultiplierPercent(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Bandit:
    return 70;
  case MonsterType::DragonSpawn:
    return 100;
  case MonsterType::Goat:
    return 100;
  case MonsterType::Goblin:
    return 120;
  case MonsterType::Golem:
    return 100;
  case MonsterType::GooBlob:
    return 100;
  case MonsterType::Gorgon:
    return 100;
  case MonsterType::MeatMan:
    return 65;
  case MonsterType::Serpent:
    return 100;
  case MonsterType::Warlock:
    return 135;
  case MonsterType::Wraith:
    return 100;
  case MonsterType::Zombie:
    return 100;
  }
}

constexpr int getPhysicalResistancePercent(MonsterType type)
{
  if (type == MonsterType::GooBlob)
    return 50;
  if (type == MonsterType::Wraith)
    return 30;
  return 0;
}

constexpr int getMagicalResistancePercent(MonsterType type)
{
  if (type == MonsterType::Goat)
    return 25;
  if (type == MonsterType::Golem)
    return 50;
  return 0;
}

MonsterTraits getTraits(MonsterType type);
