#pragma once

#include <cstdint>

class MonsterStatus
{
public:
  bool isBurning() const;
  bool isPoisoned() const;
  bool isSlowed() const;
  bool isZotted() const;
  bool isWickedSick() const;

  int getBurnStackSize() const;
  int getPoisonAmount() const;
  int getCorroded() const;

  void setBurn(uint8_t nStacks);
  void setPoison(uint16_t poisonAmount);
  void setCorroded(uint16_t numCorrosionStacks);
  void setSlowed(bool slowed);
  void setZotted();
  void setWickedSick();

private:
  enum class Status { Slowed, Zotted, WickedSick };
  uint8_t burnStackSize{0};
  uint8_t status{0};
  uint16_t poisonAmount{0};
  uint16_t corroded{0};
};
