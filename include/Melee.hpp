#pragma once

#include "Hero.hpp"
#include "Monster.hpp"

#include <optional>
#include <vector>

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
  std::optional<Hero> hero;
  std::optional<Monster> monster;
};

class Melee
{
public:
  Melee(Hero &hero, Monster &monster);
  bool heroHasInitiative();
  Outcome predictOutcome() const;

private:
  Hero &hero;
  Monster &monster;
};
