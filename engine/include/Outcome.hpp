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

inline std::string toString(Summary summary, const Debuffs& debuffs, std::optional<int> pietyChange)
{
  std::string result;
  // Only one debuff is shown in string
  if (debuffs.count(Debuff::LostDeathProtection))
  {
    if (summary == Summary::LevelUp)
      result = "Barely Level";
    else if (summary == Summary::Win)
      result = "Barely Win";
    else if (summary == Summary::Safe)
      result = "Barely Alive";
  }
  else
  {
    const std::string summaryStr = toString(summary);
    if (debuffs.count(Debuff::Cursed))
      result = summaryStr + " Cursed";
    else if (debuffs.count(Debuff::ManaBurned))
      result = summaryStr + " Mana Burn";
    else if (debuffs.count(Debuff::Poisoned))
      result = summaryStr + " Poison";
    else if (debuffs.count(Debuff::Corroded))
      result = summaryStr + " Corroded";
    else if (debuffs.count(Debuff::Weakened))
      result = summaryStr + " Weaken";
  }
  if (pietyChange.has_value())
    result += " " + std::to_string(*pietyChange) + " piety";
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
