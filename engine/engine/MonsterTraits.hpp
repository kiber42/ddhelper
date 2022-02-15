#pragma once

#include "engine/MonsterTypes.hpp"
#include "engine/StrongTypes.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>

enum class MonsterTrait : uint8_t
{
  Blinks,
  Bloodless,
  Corrosive,
  Cowardly, // TODO
  CurseBearer,
  FastRegen,
  FirstStrike,
  MagicalAttack,
  ManaBurn,
  Poisonous,
  Retaliate,
  Undead,
  Weakening,
  WickedSick,
  Zotted,

  Revives, // TODO
  Spawns,  // TODO
  Last = Spawns
};

constexpr const char* toString(MonsterTrait monsterTrait)
{
  switch (monsterTrait)
  {
  case MonsterTrait::Blinks:
    return "Blinks";
  case MonsterTrait::Bloodless:
    return "Bloodless";
  case MonsterTrait::Corrosive:
    return "Corrosive";
  case MonsterTrait::Cowardly:
    return "Cowardly";
  case MonsterTrait::CurseBearer:
    return "Curse Bearer";
  case MonsterTrait::FastRegen:
    return "Fast Regen";
  case MonsterTrait::FirstStrike:
    return "First Strike";
  case MonsterTrait::MagicalAttack:
    return "Magical Attack";
  case MonsterTrait::ManaBurn:
    return "Mana Burn";
  case MonsterTrait::Poisonous:
    return "Poisonous";
  case MonsterTrait::Retaliate:
    return "Retaliate";
  case MonsterTrait::Undead:
    return "Undead";
  case MonsterTrait::Weakening:
    return "Weakening";
  case MonsterTrait::WickedSick:
    return "Wicked Sick";
  case MonsterTrait::Zotted:
    return "Zotted";
  case MonsterTrait::Revives:
    return "Revives";
  case MonsterTrait::Spawns:
    return "Spawns";
  }
}

using Berserk = Percentage<uint8_t, struct BerserkParameter, Addable, Comparable>;
auto constexpr operator"" _berserk(unsigned long long value) { return Berserk{clampedTo<uint8_t>(value)}; }

struct MonsterTraits
{
  MonsterTraits() = default;
  MonsterTraits(MonsterType);
  MonsterTraits(std::initializer_list<MonsterTrait> traits);

  inline bool has(MonsterTrait trait) const { return traits & flag(trait); }

  DeathGaze deathGaze() const { return deathGaze_; }
  LifeSteal lifeSteal() const { return lifeSteal_; }
  Berserk berserk() const { return berserk_; }
  Knockback knockback() const { return knockback_; }

  void applyTikkiTookiBoost()
  {
    add(MonsterTrait::FirstStrike);
    add(MonsterTrait::Weakening);
  }

  void addCorrosive() { add(MonsterTrait::Corrosive); }
  void addWickedSick() { add(MonsterTrait::WickedSick); }
  void addZotted() { add(MonsterTrait::Zotted); }

protected:
  constexpr static uint32_t flag(MonsterTrait trait)
  {
    static_assert((1u << static_cast<std::underlying_type_t<MonsterTrait>>(MonsterTrait::Last)) < UINT32_MAX);
    return 1u << static_cast<std::underlying_type_t<MonsterTrait>>(trait);
  }
  inline void add(MonsterTrait trait) { traits |= flag(trait); }
  inline void toggle(MonsterTrait trait) { traits ^= flag(trait); }

  DeathGaze deathGaze_{0_deathgaze};
  // TODO: lifesteal not considered by engine
  LifeSteal lifeSteal_{0_lifesteal};
  Berserk berserk_{0_berserk};
  // TODO: knockback not considered by engine
  Knockback knockback_{0_knockback};

private:
  uint32_t traits{0};
};
