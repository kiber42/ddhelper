#pragma once

#include "Hero.hpp"
#include "Monster.hpp"

#include <ostream>
#include <set>

struct Outcome
{
  enum class Summary
  {
    Safe,
    Win,
    Death,
    LevelUp,
    NotPossible
  };
  enum class Debuff
  {
    LostDeathProtection,
    Cursed,
    ManaBurned,
    Poisoned,
    Corroded,
    Weakened,
  };
  using Debuffs = std::set<Debuff>;
  Summary summary;
  Debuffs debuffs;
  Hero hero;
  Monster monster;
};

constexpr const char* toString(Outcome::Summary status)
{
  using Summary = Outcome::Summary;
  switch (status)
  {
    case Summary::Safe: return "Safe";
    case Summary::Win: return "Win";
    case Summary::Death: return "Death";
    case Summary::LevelUp: return "Level Up";
    case Summary::NotPossible: return "Not Possible";
  }
}

constexpr const char* toString(Outcome::Debuff debuff)
{
  using Debuff = Outcome::Debuff;
  switch (debuff)
  {
    case Debuff::LostDeathProtection: return "Lost Death Protection";
    case Debuff::Cursed: return "Cursed";
    case Debuff::ManaBurned: return "Mana Burned";
    case Debuff::Poisoned: return "Poisoned";
    case Debuff::Corroded: return "Corroded";
    case Debuff::Weakened: return "Weakened";
  }
}

inline std::string toString(Outcome::Summary summary, const Outcome::Debuffs& debuffs)
{
  using Summary = Outcome::Summary;
  using Debuff = Outcome::Debuff;
  // Only one debuff is shown in string
  if (debuffs.count(Debuff::LostDeathProtection))
  {
    if (summary == Summary::Win)
      return "Barely Win";
    if (summary == Summary::Safe)
      return "Barely Alive";
  }
  if (debuffs.count(Debuff::Cursed))
    return toString(summary) + std::string(" Cursed");
  if (debuffs.count(Debuff::ManaBurned))
    return toString(summary) + std::string(" Mana Burn");
  if (debuffs.count(Debuff::Poisoned))
    return toString(summary) + std::string(" Poison");
  if (debuffs.count(Debuff::Corroded))
    return toString(summary) + std::string(" Corroded");
  if (debuffs.count(Debuff::Weakened))
    return toString(summary) + std::string(" Weaken");
  return toString(summary);
}

inline std::ostream& operator<<(std::ostream& os, const Outcome::Summary& summary)
{
  return os << toString(summary);
}

inline std::ostream& operator<<(std::ostream& os, const Outcome::Debuff& debuff)
{
  return os << toString(debuff);
}
