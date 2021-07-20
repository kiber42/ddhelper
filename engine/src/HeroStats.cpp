#include "engine/HeroStats.hpp"

#include "engine/Clamp.hpp"

#include <algorithm>
#include <cassert>

HeroStats::HeroStats()
  : HeroStats(10_HP, 10_MP, 5_damage)
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

HeroStats::HeroStats(HitPoints hpMax, ManaPoints mpMax, DamagePoints baseDamage)
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
  return hp == 0_HP;
}

HitPoints HeroStats::getHitPoints() const
{
  return hp;
}

HitPoints HeroStats::getHitPointsMax() const
{
  return hpMax;
}

void HeroStats::setHitPointsMax(HitPoints hitPointsMax)
{
  if (hitPointsMax < 1_HP)
    hitPointsMax = 1_HP;
  const bool isReduction = hitPointsMax < hpMax;
  hpMax = hitPointsMax;
  if (isReduction && hp > hpMax)
    hp = hpMax;
}

ManaPoints HeroStats::getManaPoints() const
{
  return mp;
}

ManaPoints HeroStats::getManaPointsMax() const
{
  return mpMax;
}

void HeroStats::setManaPointsMax(ManaPoints manaPointsMax)
{
  mpMax = manaPointsMax;
  if (mp > mpMax)
    mp = mpMax;
}

void HeroStats::healHitPoints(HitPoints amountPointsHealed, bool allowOverheal)
{
  const auto max = allowOverheal ? hpMax.percentage(150) : hpMax;
  if (hp < max)
    hp = std::min(hp + amountPointsHealed, max);
}

void HeroStats::loseHitPointsWithoutDeathProtection(HitPoints amountPointsLost)
{
  if (amountPointsLost < hp)
    hp -= amountPointsLost;
  else
    hp = 0_HP;
}

void HeroStats::barelySurvive()
{
  assert(hp == 0_HP);
  hp = 1_HP;
}

void HeroStats::recoverManaPoints(ManaPoints amountPointsRecovered)
{
  mp += amountPointsRecovered;
  if (mp > mpMax)
    mp = mpMax;
}

void HeroStats::loseManaPoints(ManaPoints amountPointsLost)
{
  if (amountPointsLost < mp)
    mp -= amountPointsLost;
  else
    mp = 0_MP;
}

void HeroStats::refresh()
{
  if (hp < hpMax)
    hp = hpMax;
  mp = mpMax;
}

DamagePoints HeroStats::getBaseDamage() const
{
  return baseDamage;
}

void HeroStats::setBaseDamage(DamagePoints damagePoints)
{
  if (damagePoints > 0_damage)
    baseDamage = damagePoints;
  else
    baseDamage = 1_damage;
}

DamageBonus HeroStats::getDamageBonus() const
{
  return damageBonusPercent;
}

void HeroStats::setDamageBonus(DamageBonus newDamageBonus)
{
  damageBonusPercent = newDamageBonus;
}

int HeroStats::getHealthBonus() const
{
  return healthBonus;
}

void HeroStats::addHealthBonus(Level currentLevel)
{
  ++healthBonus;
  hpMax += HitPoints{currentLevel.get()};
}

void HeroStats::addFutureHealthBonus()
{
  ++healthBonus;
}

void HeroStats::reduceHealthBonus()
{
  --healthBonus;
}
