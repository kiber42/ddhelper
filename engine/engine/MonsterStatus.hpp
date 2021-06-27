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
  bool isBurning() const;
  bool isPoisoned() const;
  bool isSlowed() const;
  bool isZotted() const;
  bool isWickedSick() const;

  BurnStackSize getBurnStackSize() const;
  PoisonAmount getPoisonAmount() const;
  CorrosionAmount getCorroded() const;

  void set(BurnStackSize nStacks);
  void set(PoisonAmount poisonAmount);
  void set(CorrosionAmount numCorrosionStacks);
  void setSlowed(bool slowed);
  void setZotted();
  void setWickedSick();

private:
  enum class Status { Slowed, Zotted, WickedSick };
  uint8_t status{0};
  BurnStackSize burnStackSize{0};
  PoisonAmount poisonAmount{0};
  CorrosionAmount corroded{0};
};
