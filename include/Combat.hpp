#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Combat
{
  // Predict outcome of melee attack
  Outcome predictOutcome(const Hero&, const Monster&);

  // Monster attacks hero (Mana Shield may result in monster HP loss)
  Outcome::Debuffs retaliate(Hero&, Monster&);

  // Predicts outcome for current monster if hero were to attack another monster
  Outcome attackOther(const Hero&, const Monster& current);

  // Predicts recovery of hero and monster when uncovering tiles
  Outcome uncoverTiles(const Hero&, const Monster&, int numTiles=1);

  // Predicts recovery of hero when uncovering tiles and no monster is around
  Hero uncoverTiles(Hero, int numTiles=1);

  // Determines outcome summary and awards experience if applicable
  Outcome::Summary summaryAndExperience(Hero& heroAfterFight, const Monster& monsterAfterFight, bool monsterWasSlowed);
} // namespace Combat
