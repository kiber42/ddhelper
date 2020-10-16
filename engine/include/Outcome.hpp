#pragma once

#include <optional>
#include <ostream>
#include <set>

enum class Summary
{
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
  std::optional<int> pietyChange;
};

constexpr const char* toString(Summary status)
{
  switch (status)
  {
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

inline std::string toString(const Outcome& outcome)
{
  std::string result;
  // Only one debuff is shown in string
  if (outcome.debuffs.count(Debuff::LostDeathProtection))
  {
    if (outcome.summary == Summary::LevelUp)
      result = "Barely Level";
    else if (outcome.summary == Summary::Win)
      result = "Barely Win";
    else if (outcome.summary == Summary::Safe)
      result = "Barely Alive";
  }
  else
  {
    const std::string summaryStr = toString(outcome.summary);
    if (outcome.debuffs.count(Debuff::Cursed))
      result = summaryStr + " Cursed";
    else if (outcome.debuffs.count(Debuff::ManaBurned))
      result = summaryStr + " Mana Burn";
    else if (outcome.debuffs.count(Debuff::Poisoned))
      result = summaryStr + " Poison";
    else if (outcome.debuffs.count(Debuff::Corroded))
      result = summaryStr + " Corroded";
    else if (outcome.debuffs.count(Debuff::Weakened))
      result = summaryStr + " Weaken";
  }
  if (outcome.pietyChange)
    result += " " + std::to_string(*outcome.pietyChange) + " piety";
  return result;
}

inline std::ostream& operator<<(std::ostream& os, const Summary& summary)
{
  return os << toString(summary);
}

inline std::ostream& operator<<(std::ostream& os, const Debuff& debuff)
{
  return os << toString(debuff);
}
