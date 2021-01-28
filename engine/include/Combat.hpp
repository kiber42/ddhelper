#pragma once

#include "Faith.hpp"
#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Combat
{
  // Perform melee attack on monster, evaluate effects on all monsters
  Summary attack(Hero&, Monster&, Monsters&);

  namespace detail
  {
    // Determines outcome summary and awards experience if applicable.
    // Helper used by Combat::attack and Magic::cast, do not call directly.
    Summary summaryAndExperience(Hero& heroAfterFight,
                                 const Monster& monsterAfterFight,
                                 bool monsterWasSlowed,
                                 bool monsterWasBurning);
  } // namespace detail
} // namespace Combat
