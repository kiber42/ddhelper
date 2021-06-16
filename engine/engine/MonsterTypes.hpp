#pragma once

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
  case MonsterType::AcidBlob:
    return 85;
  case MonsterType::AnimatedArmour:
    return 55;
  case MonsterType::Berserker:
    return 100;
  case MonsterType::BurnViper:
    return 100;
  case MonsterType::BloodSnake:
    return 100;
  case MonsterType::CaveSnake:
    return 160;
  case MonsterType::Changeling:
    return 100;
  case MonsterType::Cultist:
    return 80;
  case MonsterType::DesertTroll:
    return 140;
  case MonsterType::Djinn:
    return 110;
  case MonsterType::DoomArmour:
    return 65;
  case MonsterType::Druid:
    return 80;
  case MonsterType::ForestTroll:
    return 100;
  case MonsterType::FrozenTroll:
    return 100;
  case MonsterType::GelatinousThing:
    return 100;
  case MonsterType::Imp:
    return 80;
  case MonsterType::Illusion:
    return 100;
  case MonsterType::Minotaur:
    return 100;
  case MonsterType::MuckWalker:
    return 100;
  case MonsterType::Naga:
    return 85;
  case MonsterType::RockTroll:
    return 100;
  case MonsterType::Rusalka:
    return 100;
  case MonsterType::SteelGolem:
    return 100;
  case MonsterType::Shade:
    return 75;
  case MonsterType::SlimeBlob:
    return 100;
  case MonsterType::Thrall:
    return 110;
  case MonsterType::Tokoloshe:
    return 100;
  case MonsterType::Vampire:
    return 100;
  case MonsterType::Generic:
    return 100;
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
  case MonsterType::AcidBlob:
    return 100;
  case MonsterType::AnimatedArmour:
    return 160;
  case MonsterType::Berserker:
    return 100;
  case MonsterType::BurnViper:
    return 100;
  case MonsterType::BloodSnake:
    return 100;
  case MonsterType::CaveSnake:
    return 70;
  case MonsterType::Changeling:
    return 100;
  case MonsterType::Cultist:
    return 80;
  case MonsterType::DesertTroll:
    return 85;
  case MonsterType::Djinn:
    return 100;
  case MonsterType::DoomArmour:
    return 100;
  case MonsterType::Druid:
    return 80;
  case MonsterType::ForestTroll:
    return 85;
  case MonsterType::FrozenTroll:
    return 65;
  case MonsterType::GelatinousThing:
    return 100;
  case MonsterType::Imp:
    return 100;
  case MonsterType::Illusion:
    return 100;
  case MonsterType::Minotaur:
    return 100;
  case MonsterType::MuckWalker:
    return 100;
  case MonsterType::Naga:
    return 100;
  case MonsterType::RockTroll:
    return 100;
  case MonsterType::Rusalka:
    return 100;
  case MonsterType::SteelGolem:
    return 100;
  case MonsterType::Shade:
    return 100;
  case MonsterType::SlimeBlob:
    return 100;
  case MonsterType::Thrall:
    return 100;
  case MonsterType::Tokoloshe:
    return 100;
  case MonsterType::Vampire:
    return 100;
  case MonsterType::Generic:
    return 100;
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

inline int getDeathProtectionInitial(MonsterType type, int level)
{
  if (type == MonsterType::AnimatedArmour || type == MonsterType::DoomArmour)
    return level;
  if (type == MonsterType::Druid)
    return 1;
  return 0;
}
