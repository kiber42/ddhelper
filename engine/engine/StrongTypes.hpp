#pragma once

#include "Clamp.hpp"
#include "NamedType.hpp"

#include <cstdint>

using HitPoints = NamedType<uint16_t, struct HitPointsParameter, Addable, Subtractable, Scalable, Comparable>;
using DamagePoints = NamedType<uint16_t, struct DamagePointsParameter, Addable, Subtractable, Scalable, Comparable>;
using DeathProtection = NamedType<uint8_t, struct DeathProtectionParameter>;
using DeathGaze = Percentage<uint8_t, struct DeathGazeParameter, Addable, Comparable>;
using LifeSteal = Percentage<uint8_t, struct LifeStealParameter, Addable, Comparable>;
using Knockback = Percentage<uint16_t, struct KnockbackParameter, Addable, Comparable>;
using StoneSkinLayers = NamedType<uint8_t, struct StoneSkinLayersParameter>;
using BurnStackSize = NamedType<uint8_t, struct BurnStackSizeParameter, Addable, Scalable, Comparable>;
using PoisonAmount = NamedType<uint16_t, struct PoisonAmountParameter, Addable, Subtractable, Comparable>;
using CorrosionAmount = NamedType<uint16_t, struct CorrosionAmountParameter, Addable, Comparable>;

auto constexpr operator"" _HP(unsigned long long value) { return HitPoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _damage(unsigned long long value) { return DamagePoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _dprot(unsigned long long value) { return DeathProtection{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _deathgaze(unsigned long long value) { return DeathGaze{clampedTo<uint8_t>(value)}; }
auto constexpr operator"" _lifesteal(unsigned long long value) { return LifeSteal{clampedTo<uint8_t>(value)}; }
auto constexpr operator"" _knockback(unsigned long long value) { return Knockback{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _stoneskin(unsigned long long value) { return StoneSkinLayers{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _burn(unsigned long long value) { return BurnStackSize{clampedTo<uint8_t>(value)}; }
auto constexpr operator"" _poison(unsigned long long value) { return PoisonAmount{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _corrosion(unsigned long long value) { return CorrosionAmount{clampedTo<uint16_t>(value)}; }

class Level
{
public:
  template <class Integral>
  explicit Level(Integral level)
    : level(clamped<uint8_t>(level, 1, 10))
  {
  }

  [[nodiscard]] uint8_t& get() { return level; }
  [[nodiscard]] const uint8_t& get() const { return level; }

  Level operator++()
  {
    increase();
    return *this;
  }

  Level operator++(int)
  {
    auto current = *this;
    increase();
    return current;
  }

  bool increase()
  {
    if (level >= 10)
      return false;
    ++level;
    return true;
  }

private:
  uint8_t level;
};