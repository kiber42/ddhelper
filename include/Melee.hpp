#pragma once

#include "Hero.hpp"
#include "Monster.hpp"

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

namespace Melee
{
  Outcome predictOutcome(const Hero&, const Monster&);
} // namespace Melee
