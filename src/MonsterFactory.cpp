#include "MonsterFactory.hpp"

MonsterStats makeGenericMonsterStats(int level, int hp, int damage, int deathProtection)
{
  return MonsterStats(level, hp, hp, damage, deathProtection);
}
