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

  inline bool has(MonsterTrait trait) const
  {
    return traits & (1 << static_cast<std::underlying_type_t<MonsterTrait>>(trait));
  }

  int getDeathGazePercent() const { return deathGazePercent; }
  int getLifeStealPercent() const { return lifeStealPercent; }
  int getBerserkPercent() const { return berserkPercent; }
  int getKnockbackPercent() const { return knockbackPercent; }

  void makeFast() { add(MonsterTrait::FirstStrike); }
  void makeWeakening() { add(MonsterTrait::Weakening); }

protected:
  inline void add(MonsterTrait trait) { traits |= 1 << static_cast<std::underlying_type_t<MonsterTrait>>(trait); }
  inline void toggle(MonsterTrait trait) { traits ^= 1 << static_cast<std::underlying_type_t<MonsterTrait>>(trait); }

  uint8_t deathGazePercent{0};
  // TODO: lifesteal not considered by engine
  uint8_t lifeStealPercent{0};
  uint8_t berserkPercent{0};
  // TODO: knockback not considered by engine
  uint8_t knockbackPercent{0};

private:
  uint16_t traits{0};
};
