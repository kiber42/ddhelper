#include "Hero.hpp"

#include "Experience.hpp"
#include "Monster.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

Hero::Hero()
  : stats(1, 10, 10)
  , damage(5)
  , damageBonusPercent(0)
  , defence(0, 0, 65, 65)
  , experience()
/*
 , piety(0)
 , gold(20)
 , conversionPoints(0)
*/
{
}

int Hero::getXP() const
{
  return experience.getXP();
}

int Hero::getLevel() const
{
  return experience.getLevel();
}

int Hero::getPrestige() const
{
  return experience.getPrestige();
}

int Hero::getXPforNextLevel() const
{
  return experience.getXPforNextLevel();
}

void Hero::gainExperience(int xpGained, bool monsterWasSlowed)
{
  int level = experience.getLevel();
  int bonuses = getStatusIntensity(HeroStatus::Learning) + (monsterWasSlowed ? 1 : 0);
  experience.gain(xpGained, bonuses, hasStatus(HeroStatus::ExperienceBoost));
  removeStatus(HeroStatus::ExperienceBoost, true);
  while (getLevel() > level)
  {
    levelGainedUpdate();
    ++level;
  }
}

void Hero::gainLevel()
{
  int level = getLevel();
  experience.gainLevel();
  while (getLevel() > level)
  {
    levelGainedUpdate();
    ++level;
  }
}

void Hero::modifyLevelBy(int delta)
{
  experience.modifyLevelBy(delta);
}

bool Hero::isDefeated() const
{
  return stats.isDefeated();
}

int Hero::getHitPoints() const
{
  return stats.getHitPoints();
}

int Hero::getHitPointsMax() const
{
  return stats.getHitPointsMax();
}

int Hero::getManaPoints() const
{
  return stats.getManaPoints();
}

int Hero::getManaPointsMax() const
{
  return stats.getManaPointsMax();
}

void Hero::modifyHitPointsMax(int delta)
{
  stats.setHitPointsMax(stats.getHitPointsMax() + delta);
}

int Hero::getBaseDamage() const
{
  return std::max(damage - getStatusIntensity(HeroStatus::Weakened), 0);
}

void Hero::changeBaseDamage(int deltaDamagePoints)
{
  damage = std::max(damage + deltaDamagePoints, 0);
}

int Hero::getDamageBonusPercent() const
{
  return damageBonusPercent;
}

void Hero::changeDamageBonusPercent(int deltaDamageBonusPercent)
{
  damageBonusPercent += deltaDamageBonusPercent;
}

int Hero::getDamage() const
{
  return getBaseDamage() * (100 + getDamageBonusPercent()) / 100;
}

int Hero::getPhysicalResistPercent() const
{
  return defence.getPhysicalResistPercent();
}

int Hero::getMagicalResistPercent() const
{
  return defence.getMagicalResistPercent();
}

void Hero::setPhysicalResistPercent(int physicalResistPercent)
{
  defence.setPhysicalResistPercent(physicalResistPercent);
}

void Hero::setMagicalResistPercent(int magicalResistPercent)
{
  defence.setMagicalResistPercent(magicalResistPercent);
}

void Hero::changePhysicalResistPercent(int deltaPercent)
{
  setPhysicalResistPercent(getPhysicalResistPercent() + deltaPercent);
}

void Hero::changeMagicalResistPercent(int deltaPercent)
{
  setMagicalResistPercent(getMagicalResistPercent() + deltaPercent);
}

bool Hero::doesMagicalDamage() const
{
  return getStatusIntensity(HeroStatus::ConsecratedStrike) > 0 || getStatusIntensity(HeroStatus::MagicalAttack) > 0;
}

bool Hero::hasInitiativeVersus(const Monster& monster) const
{
    // TODO Adjust behaviour based on hero's traits
  if (hasStatus(HeroStatus::Reflexes))
    return true;

  const bool heroFast = hasStatus(HeroStatus::FirstStrike) && !hasStatus(HeroStatus::SlowStrike);
  const bool monsterFast = monster.hasFirstStrike() && !monster.isSlowed();
  if (heroFast || monsterFast)
    return !monsterFast;

  const bool heroSlow = !hasStatus(HeroStatus::FirstStrike) && hasStatus(HeroStatus::SlowStrike);
  const bool monsterSlow = monster.isSlowed();
  if (heroSlow || monsterSlow)
    return !heroSlow;

  return getLevel() > monster.getLevel();
}

int Hero::predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const
{
  return defence.predictDamageTaken(attackerDamageOutput, isMagicalDamage);
}

void Hero::takeDamage(int attackerDamageOutput, bool isMagicalDamage)
{
  stats.loseHitPoints(predictDamageTaken(attackerDamageOutput, isMagicalDamage));
}

void Hero::recover(int nSquares)
{
  if (!hasStatus(HeroStatus::Poisoned))
  {
    int multiplier = getLevel() * (hasTrait(HeroTrait::Discipline) ? 2 : 1);
    if (hasTrait(HeroTrait::Damned))
      multiplier = 1;
    // TODO: Add +1 for Bloody Sigil
    stats.healHitPoints(nSquares * multiplier, false);
  }
  if (!hasStatus(HeroStatus::ManaBurned))
  {
    stats.recoverManaPoints(nSquares);
  }
}

void Hero::healHitPoints(int amountPointsHealed, bool mayOverheal)
{
  stats.healHitPoints(amountPointsHealed, mayOverheal);
}

void Hero::loseHitPointsOutsideOfFight(int amountPointsLost)
{
  stats.loseHitPoints(amountPointsLost);
}

void Hero::recoverManaPoints(int amountPointsRecovered)
{
  stats.recoverManaPoints(amountPointsRecovered);
}

void Hero::loseManaPoints(int amountPointsLost)
{
  stats.loseManaPoints(amountPointsLost);
}

int Hero::getDeathProtection() const
{
  return stats.getDeathProtection();
}

void Hero::setDeathProtection(bool enableProtection)
{
  stats.setDeathProtection(enableProtection);
}

void Hero::addStatus(HeroStatus status, int addedIntensity)
{
  setStatusIntensity(status, statuses[status] + addedIntensity);
}

void Hero::removeStatus(HeroStatus status, bool completely)
{
  if (statuses[status] > 0)
    setStatusIntensity(status, completely ? 0 : statuses[status] - 1);
}

bool Hero::hasStatus(HeroStatus status) const
{
  return getStatusIntensity(status) > 0;
}

void Hero::setStatusIntensity(HeroStatus status, int newIntensity)
{
  const bool canStack = canHaveMultiple(status) ||
    (status == HeroStatus::Might && hasTrait(HeroTrait::Additives));
  if (newIntensity > 1 && !canStack)
    newIntensity = 1;
  if (newIntensity == statuses[status])
    return;
  statuses[status] = newIntensity;
  propagateStatus(status, newIntensity);
}

int Hero::getStatusIntensity(HeroStatus status) const
{
  auto iter = statuses.find(status);
  return iter != statuses.end() ? iter->second : 0;
}

void Hero::addTrait(HeroTrait trait)
{
  auto success = traits.insert(trait);
  if (!success.second)
  {
    assert(false);
    return;
  }

  if (trait == HeroTrait::BloodCurse)
  {
    experience.modifyLevelBy(+1);
  }
  else if (trait == HeroTrait::Humility)
  {
    experience.modifyLevelBy(-1);
  }
}

bool Hero::hasTrait(HeroTrait trait) const
{
  return traits.count(trait) > 0;
}

void Hero::propagateStatus(HeroStatus status, int intensity)
{
  switch (status)
  {
  case HeroStatus::Corrosion:
    defence.setCorrosion(intensity);
    break;
  case HeroStatus::Cursed:
    defence.setCursed(intensity > 0);
  default:
    break;
  }
}

void Hero::levelGainedUpdate()
{
  stats.setLevel(experience.getLevel());
  stats.setHitPointsMax(stats.getHitPointsMax() + 10 + stats.getHealthBonus()); // TODO
  stats.refresh();

  damage += 5;
}
