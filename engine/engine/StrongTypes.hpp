#pragma once

#include "Clamp.hpp"
#include "NamedType.hpp"

#include <cstdint>

using HitPoints = NamedType<uint16_t, struct HitPointsParameter, Addable, Subtractable, Scalable, PercentOf, Comparable>;
using ManaPoints = NamedType<uint8_t, struct ManaPointsParameter, Addable, Subtractable, PercentOf, Comparable>;
using DamagePoints = NamedType<uint16_t, struct DamagePointsParameter, Addable, Subtractable, Scalable, Comparable>;
using ExperiencePoints = NamedType<uint16_t, struct ExperiencePointsParameter, Addable, Incrementable, Subtractable, Scalable, PercentOf, Comparable>;
using DamageBonus = Percentage<int16_t, struct DamageBonusParameter, Negation, Addable, Subtractable, Comparable>;
using DeathProtection = NamedType<uint8_t, struct DeathProtectionParameter>;
using DeathGaze = Percentage<uint8_t, struct DeathGazeParameter, Addable, Comparable>;
using LifeSteal = Percentage<uint8_t, struct LifeStealParameter, Addable, Comparable>;
using Knockback = Percentage<uint16_t, struct KnockbackParameter, Addable, Comparable>;
using StoneSkinLayers = NamedType<uint8_t, struct StoneSkinLayersParameter>;
using BurnStackSize = NamedType<uint8_t, struct BurnStackSizeParameter, Addable, Scalable, Comparable>;
using PoisonAmount = NamedType<uint16_t, struct PoisonAmountParameter, Addable, Subtractable, Comparable>;
using CorrosionAmount = NamedType<uint16_t, struct CorrosionAmountParameter, Addable, Comparable>;
using DungeonMultiplier = NamedType<float, struct DungeonMultiplierParameter, Comparable>;

auto constexpr operator"" _HP(unsigned long long value) { return HitPoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _MP(unsigned long long value) { return ManaPoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _damage(unsigned long long value) { return DamagePoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _xp(unsigned long long value) { return ExperiencePoints{clampedTo<uint16_t>(value)}; }
auto constexpr operator"" _bonus(unsigned long long value) { return DamageBonus{clampedTo<int16_t>(value)}; }
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
  constexpr explicit Level(Integral level)
    : level(clamped<uint8_t>(level, 1, 10))
  {
  }

  [[nodiscard]] constexpr uint8_t& get() { return level; }
  [[nodiscard]] constexpr const uint8_t& get() const { return level; }

  constexpr Level operator++()
  {
    increase();
    return *this;
  }

  constexpr Level operator++(int)
  {
    auto current = *this;
    increase();
    return current;
  }

  constexpr bool increase()
  {
    if (level >= 10)
      return false;
    ++level;
    return true;
  }

  constexpr auto operator<=>(const Level&) const = default;

private:
  uint8_t level;
};
