#include "engine/MonsterStatus.hpp"

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

bool MonsterStatus::isZotted() const
{
  return zotted;
}

bool MonsterStatus::isWickedSick() const
{
  return wickedSick;
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

void MonsterStatus::setZotted()
{
  zotted = true;
}

void MonsterStatus::setWickedSick()
{
  wickedSick = true;
}
