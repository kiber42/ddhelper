#pragma once

#include "MonsterTypes.hpp"

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

struct MonsterTraits
{
  MonsterTraits() = default;
  MonsterTraits(MonsterType);
  MonsterTraits(std::initializer_list<MonsterTrait> traits);

  inline bool has(MonsterTrait trait) const { return traits & flag(trait); }

  uint8_t getDeathGazePercent() const { return deathGazePercent; }
  uint8_t getLifeStealPercent() const { return lifeStealPercent; }
  uint8_t getBerserkPercent() const { return berserkPercent; }
  uint8_t getKnockbackPercent() const { return knockbackPercent; }

  void applyTikkiTookiBoost()
  {
    add(MonsterTrait::FirstStrike);
    add(MonsterTrait::Weakening);
  }

  void addWickedSick() { add(MonsterTrait::WickedSick); }
  void addZotted() { add(MonsterTrait::Zotted); }

protected:
  constexpr static uint32_t flag(MonsterTrait trait)
  {
    return 1u << static_cast<std::underlying_type_t<MonsterTrait>>(trait);
  }
  inline void add(MonsterTrait trait) { traits |= flag(trait); }
  inline void toggle(MonsterTrait trait) { traits ^= flag(trait); }

  uint8_t deathGazePercent{0};
  // TODO: lifesteal not considered by engine
  uint8_t lifeStealPercent{0};
  uint8_t berserkPercent{0};
  // TODO: knockback not considered by engine
  uint8_t knockbackPercent{0};

private:
  uint32_t traits{0};
};
