#include "MonsterFactory.hpp"

// Attack and health modifiers
constexpr std::pair<int, int> getMultipliers(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Bandit:
    return {70, 100};
  case MonsterType::DragonSpawn:
    return {100, 125};
  case MonsterType::Goat:
    return {100, 90};
  case MonsterType::Goblin:
    return {120, 100};
  case MonsterType::Golem:
    return {100, 100};
  case MonsterType::GooBlob:
    return {100, 100};
  case MonsterType::Gorgon:
    return {100, 90};
  case MonsterType::MeatMan:
    return {65, 200};
  case MonsterType::Serpent:
    return {100, 100};
  case MonsterType::Warlock:
    return {135, 100};
  case MonsterType::Wraith:
    return {100, 75};
  case MonsterType::Zombie:
    return {100, 150};
  }
}

// Physical and magical resistances
constexpr std::pair<int, int> getResistances(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Goat:
    return {0, 25};
  case MonsterType::Golem:
    return {0, 50};
  case MonsterType::GooBlob:
    return {50, 0};
  case MonsterType::Wraith:
    return {30, 0};
  default:
    return {0, 0};
  }
}

MonsterTraits getTraits(MonsterType type)
{
  MonsterTraitsBuilder traits;
  switch (type)
  {
  case MonsterType::Bandit:
    traits.addCurse();
    break;
  case MonsterType::DragonSpawn:
    traits.addMagicalDamage();
    break;
  case MonsterType::Goat:
    break;
  case MonsterType::Goblin:
    traits.addFirstStrike();
    break;
  case MonsterType::Golem:
    break;
  case MonsterType::GooBlob:
    break;
  case MonsterType::Gorgon:
    traits.addFirstStrike().setDeathGazePercent(50);
    break;
  case MonsterType::MeatMan:
    break;
  case MonsterType::Serpent:
    traits.addPoisonous();
  case MonsterType::Warlock:
    traits.addMagicalDamage();
  case MonsterType::Wraith:
    traits.addUndead().addManaBurn().addMagicalDamage();
    break;
  case MonsterType::Zombie:
    traits.addUndead();
  }
  return traits.get();
}

Monster makeMonster(MonsterType type, int level, int dungeonMultiplier)
{
  std::string name = std::string(toString(type)) + " level " + std::to_string(level);
  const auto [damageMultiplier, hpMultiplier] = getMultipliers(type);
  const int hp = (level * (level + 6) - 1) * dungeonMultiplier / 100 * hpMultiplier / 100;
  const int damage = (level * (level + 5) / 2) * dungeonMultiplier / 100 * damageMultiplier / 100;
  MonsterStats stats(level, hp, hp, damage, 0);

  const auto [physicalResist, magicalResist] = getResistances(type);
  Defence defence(physicalResist, magicalResist);

  return {std::move(name), std::move(stats), std::move(defence), getTraits(type)};
}

Monster makeGenericMonster(int level, int hp, int damage)
{
  return {"Monster Level " + std::to_string(level), std::move(makeGenericMonsterStats(level, hp, damage, 0)), {}, {}};
}

MonsterStats makeGenericMonsterStats(int level, int hp, int damage, int deathProtection)
{
  return MonsterStats(level, hp, hp, damage, deathProtection);
}
