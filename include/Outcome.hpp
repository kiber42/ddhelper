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
    NotPossible
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
    case Summary::NotPossible: return "Not Possible";
  }
}

inline std::ostream& operator<<(std::ostream& os, const Outcome::Summary& summary)
{
  return os << toString(summary);
}
