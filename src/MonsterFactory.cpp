#include "MonsterFactory.hpp"

// Monster makeStandardMonster(int level) {}

Monster makeGenericMonster(int level, int hp, int damage)
{
  return {"Monster Level " + std::to_string(level), std::move(makeGenericMonsterStats(level, hp, damage, 0)), {}, {}};
}

MonsterStats makeGenericMonsterStats(int level, int hp, int damage, int deathProtection)
{
  return MonsterStats(level, hp, hp, damage, deathProtection);
}
