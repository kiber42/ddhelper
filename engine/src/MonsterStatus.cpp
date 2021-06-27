#include "engine/MonsterStatus.hpp"

#include <type_traits>

bool MonsterStatus::isBurning() const
{
  return burnStackSize > 0_burn;
}

bool MonsterStatus::isPoisoned() const
{
  return poisonAmount > 0_poison;
}

bool MonsterStatus::isSlowed() const
{
  return status & (1 << static_cast<std::underlying_type_t<Status>>(Status::Slowed));
}

bool MonsterStatus::isZotted() const
{
  return status & (1 << static_cast<std::underlying_type_t<Status>>(Status::Zotted));
}

bool MonsterStatus::isWickedSick() const
{
  return status & (1 << static_cast<std::underlying_type_t<Status>>(Status::WickedSick));
}

BurnStackSize MonsterStatus::getBurnStackSize() const
{
  return burnStackSize;
}

PoisonAmount MonsterStatus::getPoisonAmount() const
{
  return poisonAmount;
}

CorrosionAmount MonsterStatus::getCorroded() const
{
  return corroded;
}

void MonsterStatus::set(BurnStackSize nStacks)
{
  burnStackSize = nStacks;
}

void MonsterStatus::set(PoisonAmount newPoisonAmount)
{
  poisonAmount = newPoisonAmount;
}

void MonsterStatus::setSlowed(bool newSlowed)
{
  if (newSlowed)
    status |= 1 << static_cast<std::underlying_type_t<Status>>(Status::Slowed);
  else
    status &= ~(1 << static_cast<std::underlying_type_t<Status>>(Status::Slowed));
}

void MonsterStatus::set(CorrosionAmount numCorrosionStacks)
{
  corroded = numCorrosionStacks;
}

void MonsterStatus::setZotted()
{
  status |= 1 << static_cast<std::underlying_type_t<Status>>(Status::Zotted);
}

void MonsterStatus::setWickedSick()
{
  status |= 1 << static_cast<std::underlying_type_t<Status>>(Status::WickedSick);
}
