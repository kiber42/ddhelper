#pragma once

#include "StrongTypes.hpp"

enum class MonsterType : unsigned char
{
  // Basic
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
  LastBasic = Zombie,
  // Advanced
  AcidBlob,
  AnimatedArmour,
  Berserker,
  BurnViper,
  BloodSnake,
  CaveSnake,
  Changeling,
  Cultist,
  DesertTroll,
  Djinn,
  DoomArmour,
  Druid,
  ForestTroll,
  FrozenTroll,
  GelatinousThing,
  Imp,
  Illusion,
  Minotaur,
  MuckWalker,
  Naga,
  RockTroll,
  Rusalka,
  SteelGolem,
  Shade,
  SlimeBlob,
  Thrall,
  Tokoloshe,
  Vampire,
  Last = Vampire,
  // Internal
  Generic
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
  case MonsterType::AcidBlob:
    return "Acid Blob";
  case MonsterType::AnimatedArmour:
    return "Animated Armour";
  case MonsterType::Berserker:
    return "Berserker";
  case MonsterType::BurnViper:
    return "Burn Viper";
  case MonsterType::BloodSnake:
    return "Blood Snake";
  case MonsterType::CaveSnake:
    return "Cave Snake";
  case MonsterType::Changeling:
    return "Changeling";
  case MonsterType::Cultist:
    return "Cultist";
  case MonsterType::DesertTroll:
    return "Desert Troll";
  case MonsterType::Djinn:
    return "Djinn";
  case MonsterType::DoomArmour:
    return "Doom Armour";
  case MonsterType::Druid:
    return "Druid";
  case MonsterType::ForestTroll:
    return "Forest Troll";
  case MonsterType::FrozenTroll:
    return "Frozen Troll";
  case MonsterType::GelatinousThing:
    return "Gelatinous Thing";
  case MonsterType::Imp:
    return "Imp";
  case MonsterType::Illusion:
    return "Illusion";
  case MonsterType::Minotaur:
    return "Minotaur";
  case MonsterType::MuckWalker:
    return "Muck Walker";
  case MonsterType::Naga:
    return "Naga";
  case MonsterType::RockTroll:
    return "Rock Troll";
  case MonsterType::Rusalka:
    return "Rusalka";
  case MonsterType::SteelGolem:
    return "Steel Golem";
  case MonsterType::Shade:
    return "Shade";
  case MonsterType::SlimeBlob:
    return "Slime Blob";
  case MonsterType::Thrall:
    return "Thrall";
  case MonsterType::Tokoloshe:
    return "Tokoloshe";
  case MonsterType::Vampire:
    return "Vampire";
  case MonsterType::Generic:
    return "Monster";
  }
}

constexpr float getHPMultiplier(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Bandit:
    return 1;
  case MonsterType::DragonSpawn:
    return 1.25f;
  case MonsterType::Goat:
    return 0.899f;
  case MonsterType::Goblin:
    return 1;
  case MonsterType::Golem:
    return 1;
  case MonsterType::GooBlob:
    return 1;
  case MonsterType::Gorgon:
    return 0.9f;
  case MonsterType::MeatMan:
    return 2;
  case MonsterType::Serpent:
    return 1;
  case MonsterType::Warlock:
    return 1;
  case MonsterType::Wraith:
    return 0.75f;
  case MonsterType::Zombie:
    return 1.5f;
  case MonsterType::AcidBlob:
    return 0.85f;
  case MonsterType::AnimatedArmour:
    return 0.55f;
  case MonsterType::Berserker:
    return 1;
  case MonsterType::BurnViper:
    return 1;
  case MonsterType::BloodSnake:
    return 1;
  case MonsterType::CaveSnake:
    return 1.6f;
  case MonsterType::Changeling:
    return 1;
  case MonsterType::Cultist:
    return 0.8f;
  case MonsterType::DesertTroll:
    return 1.39f;
  case MonsterType::Djinn:
    return 1.1f;
  case MonsterType::DoomArmour:
    return 0.65f;
  case MonsterType::Druid:
    return 0.8f;
  case MonsterType::ForestTroll:
    return 1;
  case MonsterType::FrozenTroll:
    return 1;
  case MonsterType::GelatinousThing:
    return 1;
  case MonsterType::Imp:
    return 0.8f;
  case MonsterType::Illusion:
    return 1;
  case MonsterType::Minotaur:
    return 1;
  case MonsterType::MuckWalker:
    return 1;
  case MonsterType::Naga:
    return 0.85f;
  case MonsterType::RockTroll:
    return 1;
  case MonsterType::Rusalka:
    return 1;
  case MonsterType::SteelGolem:
    return 1;
  case MonsterType::Shade:
    return 0.75f;
  case MonsterType::SlimeBlob:
    return 1;
  case MonsterType::Thrall:
    return 1.1f;
  case MonsterType::Tokoloshe:
    return 1;
  case MonsterType::Vampire:
    return 1;
  case MonsterType::Generic:
    return 1;
  }
}

constexpr float getDamageMultiplier(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Bandit:
    return 0.7f;
  case MonsterType::DragonSpawn:
    return 1;
  case MonsterType::Goat:
    return 1;
  case MonsterType::Goblin:
    return 1.2f;
  case MonsterType::Golem:
    return 1;
  case MonsterType::GooBlob:
    return 1;
  case MonsterType::Gorgon:
    return 1;
  case MonsterType::MeatMan:
    return 0.65f;
  case MonsterType::Serpent:
    return 1;
  case MonsterType::Warlock:
    return 1.35f;
  case MonsterType::Wraith:
    return 1;
  case MonsterType::Zombie:
    return 1;
  case MonsterType::AcidBlob:
    return 1;
  case MonsterType::AnimatedArmour:
    return 1.6f;
  case MonsterType::Berserker:
    return 1;
  case MonsterType::BurnViper:
    return 1;
  case MonsterType::BloodSnake:
    return 1;
  case MonsterType::CaveSnake:
    return 0.7f;
  case MonsterType::Changeling:
    return 1;
  case MonsterType::Cultist:
    return 0.8f;
  case MonsterType::DesertTroll:
    return 0.85f;
  case MonsterType::Djinn:
    return 1;
  case MonsterType::DoomArmour:
    return 1;
  case MonsterType::Druid:
    return 0.8f;
  case MonsterType::ForestTroll:
    return 0.85f;
  case MonsterType::FrozenTroll:
    return 0.65f;
  case MonsterType::GelatinousThing:
    return 1;
  case MonsterType::Imp:
    return 1;
  case MonsterType::Illusion:
    return 1;
  case MonsterType::Minotaur:
    return 1;
  case MonsterType::MuckWalker:
    return 1;
  case MonsterType::Naga:
    return 1;
  case MonsterType::RockTroll:
    return 1;
  case MonsterType::Rusalka:
    return 1;
  case MonsterType::SteelGolem:
    return 1;
  case MonsterType::Shade:
    return 1;
  case MonsterType::SlimeBlob:
    return 1;
  case MonsterType::Thrall:
    return 1;
  case MonsterType::Tokoloshe:
    return 1;
  case MonsterType::Vampire:
    return 1;
  case MonsterType::Generic:
    return 1;
  }
}

constexpr unsigned char getPhysicalResistancePercent(MonsterType type)
{
  if (type == MonsterType::GooBlob)
    return 50;
  if (type == MonsterType::Wraith)
    return 30;
  if (type == MonsterType::FrozenTroll)
    return 50;
  if (type == MonsterType::Illusion)
    return 50;
  if (type == MonsterType::SteelGolem)
    return 25;
  if (type == MonsterType::Shade)
    return 30;
  if (type == MonsterType::Tokoloshe)
    return 50;
  return 0;
}

constexpr unsigned char getMagicalResistancePercent(MonsterType type)
{
  if (type == MonsterType::Goat)
    return 25;
  if (type == MonsterType::Golem)
    return 50;
  if (type == MonsterType::ForestTroll)
    return 25;
  if (type == MonsterType::FrozenTroll)
    return 50;
  return 0;
}

inline DeathProtection getDeathProtectionInitial(MonsterType type, Level level)
{
  if (type == MonsterType::AnimatedArmour || type == MonsterType::DoomArmour)
    return DeathProtection{level.get()};
  if (type == MonsterType::Druid)
    return DeathProtection{1};
  return DeathProtection{0};
}
