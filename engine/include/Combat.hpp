#pragma once

#include "Hero.hpp"
#include "Faith.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Combat
{
  // Perform melee attack
  Summary attack(Hero&, Monster&);

  // Pretend hero attack's another monster than the current (burn stack damage)
  Summary attackOther(Hero&, Monster& current);

  // Uncover tiles and let hero and monster recover
  Summary uncoverTiles(Hero&, Monster*, int numTiles=1);

  // Determines which debuffs the hero received due to a fight or other action
  Debuffs findDebuffs(const Hero& heroBefore, const Hero& heroAfter);

  namespace detail
  {
    // Determines outcome summary and awards experience if applicable.
    // Helper used by Combat::attack and Cast::targeted, do not call directly.
    Summary summaryAndExperience(Hero& heroAfterFight, const Monster& monsterAfterFight, bool monsterWasSlowed);

    // If the monster was defeated, applies or lifts curse
    void monsterDefeatedCurse(Hero& hero, const Monster& monster);
  }
} // namespace Combat
