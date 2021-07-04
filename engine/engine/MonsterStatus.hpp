#pragma once

#include "engine/Clamp.hpp"
#include "engine/StrongTypes.hpp"

#include <cstdint>

class MonsterStatus
{
public:
  bool isBurning() const { return burnStackSize > 0_burn; }
  bool isPoisoned() const { return poisonAmount > 0_poison; }
  bool isSlowed() const { return slowed; }

  BurnStackSize getBurnStackSize() const { return burnStackSize; }
  PoisonAmount getPoisonAmount() const { return poisonAmount; }
  CorrosionAmount getCorroded() const { return corroded; }

  void set(BurnStackSize nStacks) { burnStackSize = nStacks; }
  void set(PoisonAmount newPoisonAmount) { poisonAmount = newPoisonAmount; }
  void set(CorrosionAmount numCorrosionStacks) { corroded = numCorrosionStacks; }
  void setSlowed(bool newSlowed) { slowed = newSlowed; }

private:
  bool slowed{false};
  BurnStackSize burnStackSize{0};
  PoisonAmount poisonAmount{0};
  CorrosionAmount corroded{0};
};
