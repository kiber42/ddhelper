#pragma once

#include <optional>
#include <ostream>
#include <set>

class Hero;

enum class Summary
{
  None, // None is largely equivalent to Safe, but is not shown in description
  Safe,
  Win,
  Death,
  Petrified,
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

struct Outcome
{
  Summary summary;
  Debuffs debuffs;
  int pietyChange;
};

constexpr const char* toString(Summary status)
{
  switch (status)
  {
  case Summary::None:
    return "";
  case Summary::Safe:
    return "Safe";
  case Summary::Win:
    return "Win";
  case Summary::Death:
    return "Death";
  case Summary::Petrified:
    return "Petrified";
  case Summary::LevelUp:
    return "Level Up";
  case Summary::NotPossible:
    return "Not Possible";
  }
}

constexpr const char* toString(Debuff debuff)
{
  switch (debuff)
  {
  case Debuff::LostDeathProtection:
    return "Lost Death Protection";
  case Debuff::Cursed:
    return "Cursed";
  case Debuff::ManaBurned:
    return "Mana Burned";
  case Debuff::Poisoned:
    return "Poisoned";
  case Debuff::Corroded:
    return "Corroded";
  case Debuff::Weakened:
    return "Weakened";
  }
}

std::string toString(const Outcome& outcome);

inline std::ostream& operator<<(std::ostream& os, const Summary& summary)
{
  return os << toString(summary);
}

inline std::ostream& operator<<(std::ostream& os, const Debuff& debuff)
{
  return os << toString(debuff);
}

inline bool operator==(const Outcome& left, const Outcome& right)
{
  return left.summary == right.summary && left.debuffs == right.debuffs && left.pietyChange == right.pietyChange;
}

// Determines which debuffs the hero received due to a fight or other action
Debuffs findDebuffs(const Hero& heroBefore, const Hero& heroAfter);
