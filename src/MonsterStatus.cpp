#include "MonsterStatus.hpp"

MonsterStatus::MonsterStatus()
  : burnStackSize(0)
  , poisonAmount(0)
  , slowed(false)
  , corroded(0)
  , weakened(0)
{
}

bool MonsterStatus::isBurning() const
{
  return burnStackSize > 0;
}

bool MonsterStatus::isPoisoned() const
{
  return poisonAmount > 0;
}

bool MonsterStatus::isSlowed() const
{
  return slowed;
}

int MonsterStatus::getBurnStackSize() const
{
  return burnStackSize;
}

int MonsterStatus::getPoisonAmount() const
{
  return poisonAmount;
}

int MonsterStatus::getCorroded() const
{
  return corroded;
}

int MonsterStatus::getWeakened() const
{
  return weakened;
}

void MonsterStatus::setBurn(int nStacks)
{
  burnStackSize = nStacks;
}

void MonsterStatus::setPoison(int newPoisonAmount)
{
  poisonAmount = newPoisonAmount;
}

void MonsterStatus::setSlowed(bool newStunned)
{
  slowed = newStunned;
}

void MonsterStatus::setCorroded(int numCorrosionStacks)
{
  corroded = numCorrosionStacks;
}

void MonsterStatus::setWeakened(int numWeakened)
{
  weakened = numWeakened;
}
