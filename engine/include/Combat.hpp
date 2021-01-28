#pragma once

#include "Faith.hpp"
#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Combat
{
  // Perform melee attack
  Summary attack(Hero&, Monster&);

  // Pretend hero attacks another monster than the current (burn stack damage)
  Summary attackOther(Hero&, Monster& current);

  namespace detail
  {
    // Determines outcome summary and awards experience if applicable.
    // Helper used by Combat::attack and Magic::cast, do not call directly.
    Summary summaryAndExperience(Hero& heroAfterFight,
                                 const Monster& monsterAfterFight,
                                 bool monsterWasSlowed,
                                 bool monsterWasBurning);

    // If the monster was defeated, applies or lifts curse
    void monsterDefeatedCurse(Hero& hero, const Monster& monster);
  } // namespace detail
} // namespace Combat
