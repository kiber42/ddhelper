#include "HeroStats.hpp"

HeroStats::HeroStats(int level, int hpMax, int mpMax)
  : stats(level, hpMax, mpMax, 0)
  , healthBonus(0)
{
}

int HeroStats::getLevel() const
{
  return stats.getLevel();
}

void HeroStats::setLevel(int level)
{
  stats.setLevel(level);
}

bool HeroStats::isDefeated() const
{
  return stats.isDefeated();
}

int HeroStats::getHitPoints() const
{
  return stats.getHitPoints();
}

int HeroStats::getHitPointsMax() const
{
  return stats.getHitPointsMax();
}

void HeroStats::setHitPointsMax(int hitPointsMax)
{
  stats.setHitPointsMax(hitPointsMax, true);
}

int HeroStats::getManaPoints() const
{
  return stats.getManaPoints();
}

int HeroStats::getManaPointsMax() const
{
  return stats.getManaPointsMax();
}

void HeroStats::setManaPointsMax(int manaPointsMax)
{
  stats.setManaPointsMax(manaPointsMax);
}

int HeroStats::getHealthBonus() const
{
  return healthBonus;
}

void HeroStats::addHealthBonus()
{
  healthBonus += 1;
  stats.setHitPointsMax(stats.getHitPointsMax() + stats.getLevel(), false);
}

void HeroStats::healHitPoints(int amountPointsHealed, bool allowOverheal)
{
  stats.healHitPoints(amountPointsHealed, allowOverheal);
}

void HeroStats::loseHitPoints(int amountPointsLost)
{
  stats.loseHitPoints(amountPointsLost);
}

void HeroStats::recoverManaPoints(int amountPointsRecovered)
{
  stats.recoverManaPoints(amountPointsRecovered);
}

void HeroStats::loseManaPoints(int amountPointsLost)
{
  stats.loseManaPoints(amountPointsLost);
}

void HeroStats::refresh()
{
  stats.refresh();
}

int HeroStats::getDeathProtection() const
{
  return stats.getDeathProtection();
}

void HeroStats::setDeathProtection(bool enableProtection)
{
  stats.setDeathProtection(enableProtection ? 1 : 0);
}
