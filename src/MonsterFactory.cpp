#include "MonsterFactory.hpp"

// Attack and health modifiers, in percent
constexpr std::pair<int, int> getMultipliers(MonsterType type)
{
  switch (type)
  {
  case MonsterType::Bandit:
    return {100, 70};
  case MonsterType::DragonSpawn:
    return {125, 100};
  case MonsterType::Goat:
    return {90, 100};
  case MonsterType::Goblin:
    return {100, 120};
  case MonsterType::Golem:
    return {100, 100};
  case MonsterType::GooBlob:
    return {100, 100};
  case MonsterType::Gorgon:
    return {90, 100};
  case MonsterType::MeatMan:
    return {200, 65};
  case MonsterType::Serpent:
    return {100, 100};
  case MonsterType::Warlock:
    return {100, 135};
  case MonsterType::Wraith:
    return {75, 100};
  case MonsterType::Zombie:
    return {150, 100};
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
  const auto [hpMultiplier, damageMultiplier] = getMultipliers(type);
  // HP sometimes appears to be 1 too high
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
