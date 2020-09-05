#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Melee
{
  Outcome predictOutcome(const Hero&, const Monster&);

  Outcome::Debuffs retaliate(Hero&, const Monster&);
} // namespace Melee
