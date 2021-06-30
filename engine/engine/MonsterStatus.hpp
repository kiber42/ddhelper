#pragma once

#include "Clamp.hpp"
#include "NamedType.hpp"

#include <cstdint>

using BurnStackSize = NamedType<uint8_t, struct BurnStackSizeParameter, Addable, Scalable, Comparable>;
using PoisonAmount = NamedType<uint16_t, struct PoisonAmountParameter, Addable, Subtractable, Comparable>;
using CorrosionAmount = NamedType<uint16_t, struct CorrosionAmountParameter, Addable, Comparable>;

auto constexpr operator"" _burn(unsigned long long value) { return BurnStackSize{clampedTo<uint8_t>(value)}; }
auto constexpr operator"" _poison(unsigned long long value) { return PoisonAmount{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _corrosion(unsigned long long value) { return CorrosionAmount{clampedTo<uint16_t>(value)}; }

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
