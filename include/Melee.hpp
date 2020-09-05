#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Melee
{
  Outcome predictOutcome(const Hero&, const Monster&);

  Outcome::Debuffs retaliate(Hero&, const Monster&);

  // Predicts outcome for current monster if hero were to attack another monster
  Outcome attackOther(const Hero&, const Monster& current);

  // Predicts recovery of hero and monster when uncovering tiles
  Outcome uncoverTiles(const Hero&, const Monster&, int numTiles=1);

  // Predicts recovery of hero when uncovering tiles and no monster is around
  Hero uncoverTiles(Hero, int numTiles=1);
} // namespace Melee
