#include "MonsterStats.hpp"

#include <algorithm>

MonsterStats::MonsterStats(int level, int hp, int hpMax, int damage, int deathProtection)
  : level(level)
  , hp(hp)
  , hpMax(hpMax)
  , damage(damage)
  , deathProtection(deathProtection)
{
}

int MonsterStats::getLevel() const
{
  return level;
}

bool MonsterStats::isDefeated() const
{
  return hp == 0;
}

int MonsterStats::getHitPoints() const
{
  return hp;
}

int MonsterStats::getHitPointsMax() const
{
  return hpMax;
}

void MonsterStats::healHitPoints(int amountPointsHealed, bool allowOverheal)
{
  const int max = allowOverheal ? hpMax * 3 / 2 : hpMax;
  hp = std::min(hp + amountPointsHealed, max);
}

void MonsterStats::loseHitPoints(int amountPointsLost)
{
  hp = std::max(hp - amountPointsLost, 0);
  if (hp == 0 && deathProtection > 0)
  {
    hp = 1;
    --deathProtection;
  }
}

int MonsterStats::getDamage() const
{
  return damage;
}

void MonsterStats::setDamage(int damagePoints)
{
  damage = damagePoints;
}

int MonsterStats::getDeathProtection() const
{
  return deathProtection;
}

void MonsterStats::setDeathProtection(int numDeathProtectionLayers)
{
  deathProtection = numDeathProtectionLayers;
}
