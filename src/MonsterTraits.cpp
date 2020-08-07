#include "MonsterTraits.hpp"

MonsterTraits::MonsterTraits()
  : magicalDamage(false)
  , retaliate(false)
  , poisonous(false)
  , manaBurn(false)
  , curse(false)
  , corrosive(false)
  , weakening(false)
  , firstStrike(false)
  , deathGazePercent(0)
  , lifeStealPercent(0)
  , undead(false)
  , bloodless(false)
{
}

MonsterTraits& MonsterTraits::addMagicalDamage()
{
  magicalDamage = true;
  return *this;
}

MonsterTraits& MonsterTraits::addRetaliate()
{
  retaliate = true;
  return *this;
}

MonsterTraits& MonsterTraits::addPoisonous()
{
  poisonous = true;
  return *this;
}

MonsterTraits& MonsterTraits::addManaBurn()
{
  manaBurn = true;
  return *this;
}

MonsterTraits& MonsterTraits::addCurse()
{
  curse = true;
  return *this;
}

MonsterTraits& MonsterTraits::addCorrosive()
{
  corrosive = true;
  return *this;
}

MonsterTraits& MonsterTraits::addWeakening()
{
  weakening = true;
  return *this;
}

MonsterTraits& MonsterTraits::addFirstStrike()
{
  firstStrike = true;
  return *this;
}

MonsterTraits& MonsterTraits::setDeathGazePercent(int newDeathGazePercent)
{
  deathGazePercent = newDeathGazePercent;
  return *this;
}

MonsterTraits& MonsterTraits::setLifeStealPercent(int newLifeStealPercent)
{
  lifeStealPercent = newLifeStealPercent;
  return *this;
}

MonsterTraits& MonsterTraits::addUndead()
{
  undead = true;
  return *this;
}

MonsterTraits& MonsterTraits::addBloodless()
{
  bloodless = true;
  return *this;
}

MonsterTraits& MonsterTraits::removeMagicalDamage()
{
  magicalDamage = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeRetaliate()
{
  retaliate = false;
  return *this;
}

MonsterTraits& MonsterTraits::removePoisonous()
{
  poisonous = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeManaBurn()
{
  manaBurn = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeCurse()
{
  curse = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeCorrosive()
{
  corrosive = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeWeakening()
{
  weakening = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeFirstStrike()
{
  firstStrike = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeUndead()
{
  undead = false;
  return *this;
}

MonsterTraits& MonsterTraits::removeBloodless()
{
  bloodless = false;
  return *this;
}

bool MonsterTraits::doesMagicalDamage() const
{
  return magicalDamage;
}

bool MonsterTraits::doesRetaliate() const
{
  return retaliate;
}

bool MonsterTraits::isPoisonous() const
{
  return poisonous;
}
bool MonsterTraits::hasManaBurn() const
{
  return manaBurn;
}

bool MonsterTraits::bearsCurse() const
{
  return curse;
}

bool MonsterTraits::isCorrosive() const
{
  return corrosive;
}

bool MonsterTraits::isWeakening() const
{
  return weakening;
}

bool MonsterTraits::hasFirstStrike() const
{
  return firstStrike;
}

int MonsterTraits::getDeathGazePercent() const
{
  return deathGazePercent;
}

int MonsterTraits::getLifeStealPercent() const
{
  return lifeStealPercent;
}

bool MonsterTraits::isUndead() const
{
  return undead;
}

bool MonsterTraits::isBloodless() const
{
  return bloodless;
}
