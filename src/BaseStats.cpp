#include "BaseStats.hpp"

#include <algorithm>
#include <cassert>

BaseStats::BaseStats(int level, int hpMax, int mpMax, int damage)
  : level(level)
  , hp(hpMax)
  , hpMax(hpMax)
  , mp(mpMax)
  , mpMax(mpMax)
  , damage(damage)
  , deathProtection(0)
{
}

int BaseStats::getLevel() const
{
  return level;
}

void BaseStats::setLevel(int newLevel)
{
  level = newLevel;
}

bool BaseStats::isDefeated() const
{
  return getHitPoints() <= 0;
}

int BaseStats::getHitPoints() const
{
  return hp;
}

int BaseStats::getHitPointsMax() const
{
  return hpMax;
}

void BaseStats::setHitPointsMax(int hitPointsMax)
{
  hpMax = hitPointsMax;
}

int BaseStats::getManaPoints() const
{
  return mp;
}

int BaseStats::getManaPointsMax() const
{
  return mpMax;
}

void BaseStats::setManaPointsMax(int manaPointsMax)
{
  mpMax = manaPointsMax;
}

int BaseStats::getDamage() const
{
  return damage;
}

void BaseStats::setDamage(int newDamage)
{
  damage = newDamage;
}

void BaseStats::healHitPoints(int amountPointsHealed, bool allowOverheal)
{
  int max = hpMax;
  if (allowOverheal)
    max = hpMax * 150 / 100;
  hp = std::min(hp + amountPointsHealed, max);
}

void BaseStats::loseHitPoints(int amountPointsLost)
{
  hp = std::max(hp - amountPointsLost, 0);
  if (hp == 0 && deathProtection > 0)
  {
    hp = 1;
    --deathProtection;
  }
}

void BaseStats::recoverManaPoints(int amountPointsRecovered)
{
  mp = std::min(mp + amountPointsRecovered, mpMax);
}

void BaseStats::loseManaPoints(int amountPointsLost)
{
  assert(amountPointsLost <= mp);
  mp = std::max(mp - amountPointsLost, 0);
}

void BaseStats::refresh()
{
  hp = hpMax;
  mp = mpMax;
}

int BaseStats::getDeathProtection() const
{
  return deathProtection;
}

void BaseStats::setDeathProtection(int numDeathProtectionLayers)
{
  deathProtection = numDeathProtectionLayers;
}
