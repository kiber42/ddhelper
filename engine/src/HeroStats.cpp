#include "engine/HeroStats.hpp"

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

HeroStats::HeroStats(RegalSize)
  : hp(8)
  , hpMax(8)
  , mp(8)
  , mpMax(8)
  , baseDamage(1)
  , damageBonusPercent(0)
  , healthBonus(-2)
{
}

HeroStats::HeroStats(uint16_t hpMax, uint16_t mpMax, uint16_t baseDamage)
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

uint16_t HeroStats::getHitPoints() const
{
  return hp;
}

uint16_t HeroStats::getHitPointsMax() const
{
  return hpMax;
}

void HeroStats::setHitPointsMax(uint16_t hitPointsMax)
{
  if (hitPointsMax < 1)
    hitPointsMax = 1;
  const bool isReduction = hitPointsMax < hpMax;
  hpMax = hitPointsMax;
  if (isReduction && hp > hpMax)
    hp = hpMax;
}

uint16_t HeroStats::getManaPoints() const
{
  return mp;
}

uint16_t HeroStats::getManaPointsMax() const
{
  return mpMax;
}

void HeroStats::setManaPointsMax(uint16_t manaPointsMax)
{
  mpMax = manaPointsMax;
  if (mp > mpMax)
    mp = mpMax;
}

void HeroStats::healHitPoints(uint16_t amountPointsHealed, bool allowOverheal)
{
  const auto max = allowOverheal ? hpMax * 3 / 2 : hpMax;
  if (hp < max)
    hp = std::min(hp + amountPointsHealed, max);
}

void HeroStats::loseHitPointsWithoutDeathProtection(uint16_t amountPointsLost)
{
  if (amountPointsLost < hp)
    hp -= amountPointsLost;
  else
    hp = 0;
}

void HeroStats::barelySurvive()
{
  assert(hp == 0);
  hp = 1;
}

void HeroStats::recoverManaPoints(uint16_t amountPointsRecovered)
{
  mp += amountPointsRecovered;
  if (mp > mpMax)
    mp = mpMax;
}

void HeroStats::loseManaPoints(uint16_t amountPointsLost)
{
  if (amountPointsLost < mp)
    mp -= amountPointsLost;
  else
    mp = 0;
}

void HeroStats::refresh()
{
  if (hp < hpMax)
    hp = hpMax;
  mp = mpMax;
}

uint16_t HeroStats::getBaseDamage() const
{
  return baseDamage;
}

void HeroStats::setBaseDamage(uint16_t damagePoints)
{
  baseDamage = damagePoints;
}

int16_t HeroStats::getDamageBonusPercent() const
{
  return damageBonusPercent;
}

void HeroStats::setDamageBonusPercent(int16_t newDamageBonus)
{
  damageBonusPercent = newDamageBonus;
}

int HeroStats::getHealthBonus() const
{
  return healthBonus;
}

void HeroStats::addHealthBonus(uint8_t unmodifiedLevel)
{
  ++healthBonus;
  hpMax += unmodifiedLevel;
}

void HeroStats::reduceHealthBonus()
{
  --healthBonus;
}
