#pragma once

#include "Hero.hpp"
#include "Monster.hpp"

#include <ostream>

struct Outcome
{
  enum class Summary
  {
    Safe,
    HeroWins,
    HeroDefeated,
    HeroDebuffed,
    Error
  };
  Summary summary;
  Hero hero;
  Monster monster;
};

constexpr const char* toString(Outcome::Summary status)
{
  using Summary = Outcome::Summary;
  switch (status)
  {
    case Summary::Safe: return "Safe";
    case Summary::HeroWins: return "Win";
    case Summary::HeroDefeated: return "Death";
    case Summary::HeroDebuffed: return "Debuffed";
    default:
    case Summary::Error: return "ERROR";
  }
}

inline std::ostream& operator<<(std::ostream& os, const Outcome::Summary& summary)
{
  return os << toString(summary);
}

namespace Melee
{
  Outcome predictOutcome(const Hero&, const Monster&);
} // namespace Melee
