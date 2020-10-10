#include "HeroStats.hpp"

#include <algorithm>
#include <cassert>

HeroStats::HeroStats()
  : HeroStats(10, 10, 5)
{
}

HeroStats::HeroStats(IsDangerous)
  : hp(5)
  , hpMax(5)
  , mp(10)
  , mpMax(10)
  , baseDamage(5)
  , damageBonusPercent(40)
  , healthBonus(-5)
{
}

HeroStats::HeroStats(int hpMax, int mpMax, int baseDamage)
  : hp(hpMax)
  , hpMax(hpMax)
  , mp(mpMax)
  , mpMax(mpMax)
  , baseDamage(baseDamage)
  , damageBonusPercent(0)
  , healthBonus(0)
{
}

bool HeroStats::isDefeated() const
{
  return hp == 0;
}

int HeroStats::getHitPoints() const
{
  return hp;
}

int HeroStats::getHitPointsMax() const
{
  return hpMax;
}

void HeroStats::setHitPointsMax(int hitPointsMax)
{
  if (hitPointsMax < 1)
    hitPointsMax = 1;
  const bool isReduction = hitPointsMax < hpMax;
  hpMax = hitPointsMax;
  if (isReduction && hp > hpMax)
    hp = hpMax;
}

int HeroStats::getManaPoints() const
{
  return mp;
}

int HeroStats::getManaPointsMax() const
{
  return mpMax;
}

void HeroStats::setManaPointsMax(int manaPointsMax)
{
  mpMax = manaPointsMax;
  if (mp > mpMax)
    mp = mpMax;
}

void HeroStats::healHitPoints(int amountPointsHealed, bool allowOverheal)
{
  const int max = allowOverheal ? hpMax * 3 / 2 : hpMax;
  hp = std::min(hp + amountPointsHealed, max);
}

void HeroStats::loseHitPointsWithoutDeathProtection(int amountPointsLost)
{
  hp = std::max(hp - amountPointsLost, 0);
}

void HeroStats::barelySurvive()
{
  assert(hp == 0);
  hp = 1;
}

void HeroStats::recoverManaPoints(int amountPointsRecovered)
{
  mp = std::min(mp + amountPointsRecovered, mpMax);
}

void HeroStats::loseManaPoints(int amountPointsLost)
{
  assert(amountPointsLost <= mp);
  mp = std::max(mp - amountPointsLost, 0);
}

void HeroStats::refresh()
{
  hp = hpMax;
  mp = mpMax;
}

int HeroStats::getBaseDamage() const
{
  return baseDamage;
}

void HeroStats::setBaseDamage(int damagePoints)
{
  baseDamage = damagePoints;
}

int HeroStats::getDamageBonusPercent() const
{
  return damageBonusPercent;
}

void HeroStats::setDamageBonusPercent(int newDamageBonus)
{
  damageBonusPercent = newDamageBonus;
}

int HeroStats::getHealthBonus() const
{
  return healthBonus;
}

void HeroStats::addHealthBonus(int unmodifiedLevel)
{
  healthBonus += 1;
  hpMax += unmodifiedLevel;
}
