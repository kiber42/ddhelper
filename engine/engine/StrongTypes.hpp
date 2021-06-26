#pragma once

#include "Clamp.hpp"
#include "NamedType.hpp"

#include <cstdint>

using HitPoints = NamedType<uint16_t, struct HitPointsParameter, Addable, Subtractable, Comparable>;
using DamagePoints = NamedType<uint16_t, struct DamagePointsParameter>;
using DeathProtection = NamedType<uint8_t, struct DeathProtectionParameter>;
using DungeonMultiplier = NamedType<uint8_t, struct DungeonMultiplierParameter>;

auto constexpr operator"" _HP(unsigned long long value) { return HitPoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _damage(unsigned long long value) { return DamagePoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _dprot(unsigned long long value) { return DeathProtection{clampedTo<uint16_t>(value)}; }

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
