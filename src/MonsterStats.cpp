#include "MonsterStats.hpp"

MonsterStats::MonsterStats(int level, int hp, int hpMax, int damage, int deathProtection)
  : stats(level, hpMax, 0, damage)
{
  if (hp < hpMax)
    stats.loseHitPoints(hpMax - hp);
  stats.setDeathProtection(deathProtection);
}

int MonsterStats::getLevel() const
{
  return stats.getLevel();
}

bool MonsterStats::isDefeated() const
{
  return stats.isDefeated();
}

int MonsterStats::getHitPoints() const
{
  return stats.getHitPoints();
}

int MonsterStats::getHitPointsMax() const
{
  return stats.getHitPointsMax();
}

int MonsterStats::getDamage() const
{
  return stats.getDamage();
}

void MonsterStats::setDamage(int damagePoints)
{
  stats.setDamage(damagePoints);
}

void MonsterStats::healHitPoints(int amountPointsHealed, bool allowOverheal)
{
  stats.healHitPoints(amountPointsHealed, allowOverheal);
}

void MonsterStats::loseHitPoints(int amountPointsLost)
{
  stats.loseHitPoints(amountPointsLost);
}

int MonsterStats::getDeathProtection() const
{
  return stats.getDeathProtection();
}

void MonsterStats::setDeathProtection(int numDeathProtectionLayers)
{
  stats.setDeathProtection(numDeathProtectionLayers);
}
