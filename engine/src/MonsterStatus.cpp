#include "engine/MonsterStatus.hpp"

#include <type_traits>

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

void MonsterStatus::setBurn(uint8_t nStacks)
{
  burnStackSize = nStacks;
}

void MonsterStatus::setPoison(uint16_t newPoisonAmount)
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

void MonsterStatus::setCorroded(uint16_t numCorrosionStacks)
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
