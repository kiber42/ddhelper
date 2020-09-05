#include "Monster.hpp"

#include <algorithm>
#include <utility>

Monster::Monster(MonsterStats stats, Defence damage, MonsterTraits traits)
  : stats(std::move(stats))
  , defence(std::move(damage))
  , status{}
  , traits(std::move(traits))
{
  defence.setCorrosion(getCorroded());
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
  return defence.predictDamageTaken(attackerDamageOutput, isMagicalDamage) + status.getBurnStackSize();
}

void Monster::takeDamage(int attackerDamageOutput, bool isMagicalDamage)
{
  stats.loseHitPoints(predictDamageTaken(attackerDamageOutput, isMagicalDamage));
  status.setStunned(false);
  status.setBurn(0);
}

void Monster::takeFireballDamage(int casterLevel, bool wearsRing)
{
  int damagePoints = casterLevel * (wearsRing ? 5 : 4);
  stats.loseHitPoints(predictDamageTaken(damagePoints, true));
  status.setStunned(false);
  burn(casterLevel * 2);
}

void Monster::recover(int nSquares)
{
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
    status.setStunned(false);
    stats.loseHitPoints(status.getBurnStackSize());
    status.setBurn(0);
  }
}

void Monster::poison(int addedPoisonAmount)
{
  status.setPoison(status.getPoisonAmount() + addedPoisonAmount);
}

void Monster::stun()
{
  status.setStunned(true);
}

void Monster::erodeResitances()
{
  defence.setPhysicalResistPercent(defence.getPhysicalResistPercent() - 3);
  defence.setMagicalResistPercent(defence.getMagicalResistPercent() - 3);
}

void Monster::petrify()
{
  status.setBurn(false);
  status.setStunned(false);
  stats.setDeathProtection(0);
  stats.loseHitPoints(stats.getHitPoints());
}

bool Monster::isBurning() const
{
  return status.isBurning();
}

bool Monster::isPoisoned() const
{
  return status.isPoisoned();
}

bool Monster::isStunned() const
{
  return status.isStunned();
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

void Monster::corrode()
{
  status.setCorroded(getCorroded() + 1);
  defence.setCorrosion(getCorroded());
}

void Monster::weaken()
{
  status.setWeakened(getWeakened() + 1);
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
