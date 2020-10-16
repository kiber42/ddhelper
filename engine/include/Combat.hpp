#pragma once

#include "Hero.hpp"
#include "Faith.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Combat
{
  // Predict outcome of melee attack
  Summary attack(Hero&, Monster&);

  // Predicts outcome for current monster if hero were to attack another monster
  Summary attackOther(Hero&, Monster& current);

  // Predicts recovery of hero and monster when uncovering tiles
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
