#include "Monster.hpp"

#include <algorithm>
#include <utility>

#include "MonsterTypes.hpp"

int Monster::lastId = 0;

Monster::Monster(MonsterType type, int level, int dungeonMultiplier)
  : name(std::string(toString(type)) + " level " + std::to_string(level))
  , id(++lastId)
  , stats(type, level, dungeonMultiplier)
  , defence(type)
  , traits(type)
{
}

Monster::Monster(std::string name, MonsterStats stats, Defence damage, MonsterTraits traits)
  : name(std::move(name))
  , id(++lastId)
  , stats(std::move(stats))
  , defence(std::move(damage))
  , status{}
  , traits(std::move(traits))
{
}

Monster::Monster(int level, int hp, int damage)
  : Monster("Monster Level " + std::to_string(level), {level, hp, damage, 0}, {}, {})
{
}

const char* Monster::getName() const
{
  return name.c_str();
}

int Monster::getID() const
{
  return id;
}

int Monster::getLevel() const
{
  return stats.getLevel();
}

bool Monster::isDefeated() const
{
  return stats.isDefeated();
}

int Monster::getHitPoints() const
{
  return stats.getHitPoints();
}

int Monster::getHitPointsMax() const
{
  return stats.getHitPointsMax();
}

int Monster::getDamage() const
{
  return stats.getDamage();
}

int Monster::getPhysicalResistPercent() const
{
  return defence.getPhysicalResistPercent();
}

int Monster::getMagicalResistPercent() const
{
  return defence.getMagicalResistPercent();
}

int Monster::predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const
{
  return defence.predictDamageTaken(attackerDamageOutput, isMagicalDamage, status.getBurnStackSize());
}

void Monster::takeDamage(int attackerDamageOutput, bool isMagicalDamage)
{
  stats.loseHitPoints(predictDamageTaken(attackerDamageOutput, isMagicalDamage));
  status.setSlowed(false);
  status.setBurn(0);
}

void Monster::takeFireballDamage(int casterLevel, int damageMultiplier)
{
  const int damagePoints = casterLevel * damageMultiplier;
  stats.loseHitPoints(predictDamageTaken(damagePoints, true));
  status.setSlowed(false);
  burn(casterLevel * 2);
}

void Monster::takeManaShieldDamage(int casterLevel)
{
  // Mana Shield damage against magical resistance is rounded down (usually resisted damage is rounded down)
  stats.loseHitPoints(casterLevel * (100 - getMagicalResistPercent()) / 100);
}

void Monster::receiveCrushingBlow()
{
  int delta = stats.getHitPoints() - stats.getHitPointsMax() * 3 / 4;
  if (delta > 0)
    stats.loseHitPoints(delta);
  status.setSlowed(false);
  status.setBurn(0);
}

void Monster::recover(int nSquares)
{
  if (isDefeated())
    return;
  int recoverPoints = nSquares * (getLevel() - static_cast<int>(status.isBurning()));
  if (status.isPoisoned())
  {
    int poison = status.getPoisonAmount();
    if (poison >= recoverPoints)
    {
      status.setPoison(poison - recoverPoints);
      recoverPoints = 0;
    }
    else
    {
      recoverPoints -= poison;
      status.setPoison(0);
    }
  }
  if (recoverPoints > 0)
    stats.healHitPoints(recoverPoints, false);
}

void Monster::burn(int nMaxStacks)
{
  status.setBurn(std::min(status.getBurnStackSize() + 1, nMaxStacks));
}

void Monster::burnMax(int nMaxStacks)
{
  status.setBurn(nMaxStacks);
}

void Monster::burnDown()
{
  if (status.getBurnStackSize() > 0)
  {
    status.setSlowed(false);
    stats.loseHitPoints(status.getBurnStackSize());
    status.setBurn(0);
  }
}

bool Monster::poison(int addedPoisonAmount)
{
  if (isUndead())
    return false;
  status.setPoison(status.getPoisonAmount() + addedPoisonAmount);
  return true;
}

void Monster::slow()
{
  status.setSlowed(true);
}

void Monster::erodeResitances()
{
  defence.setPhysicalResistPercent(defence.getPhysicalResistPercent() - 3);
  defence.setMagicalResistPercent(defence.getMagicalResistPercent() - 3);
}

void Monster::petrify()
{
  die();
}

void Monster::die()
{
  status.setBurn(false);
  status.setSlowed(false);
  stats.setDeathProtection(0);
  stats.loseHitPoints(stats.getHitPoints());
}

void Monster::corrode()
{
  status.setCorroded(getCorroded() + 1);
  defence.setCorrosion(getCorroded());
}

void Monster::weaken()
{
  status.setWeakened(getWeakened() + 1);
}

bool Monster::isBurning() const
{
  return status.isBurning();
}

bool Monster::isPoisoned() const
{
  return status.isPoisoned();
}

bool Monster::isSlowed() const
{
  return status.isSlowed();
}

int Monster::getBurnStackSize() const
{
  return status.getBurnStackSize();
}

int Monster::getPoisonAmount() const
{
  return status.getPoisonAmount();
}

int Monster::getDeathProtection() const
{
  return stats.getDeathProtection();
}

int Monster::getCorroded() const
{
  return status.getCorroded();
}

int Monster::getWeakened() const
{
  return status.getWeakened();
}

bool Monster::doesMagicalDamage() const
{
  return traits.doesMagicalDamage();
}

bool Monster::doesRetaliate() const
{
  return traits.doesRetaliate();
}

bool Monster::isPoisonous() const
{
  return traits.isPoisonous();
}

bool Monster::hasManaBurn() const
{
  return traits.hasManaBurn();
}

bool Monster::bearsCurse() const
{
  return traits.bearsCurse();
}

bool Monster::isCorrosive() const
{
  return traits.isCorrosive();
}

bool Monster::isWeakening() const
{
  return traits.isWeakening();
}

bool Monster::hasFirstStrike() const
{
  return traits.hasFirstStrike();
}

int Monster::getDeathGazePercent() const
{
  return traits.getDeathGazePercent();
}

int Monster::getLifeStealPercent() const
{
  return traits.getLifeStealPercent();
}

bool Monster::isUndead() const
{
  return traits.isUndead();
}

bool Monster::isBloodless() const
{
  return traits.isBloodless();
}
