#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

namespace Combat
{
  // Predict outcome of melee attack
  Summary predictOutcome(Hero&, Monster&);

  // Predicts outcome for current monster if hero were to attack another monster
  Summary attackOther(Hero&, Monster& current);

  // Predicts recovery of hero and monster when uncovering tiles
  Summary uncoverTiles(Hero&, std::optional<Monster>&, int numTiles=1);

  // Determines which debuffs the hero received due to a fight or other action
  Debuffs findDebuffs(const Hero& heroBefore, const Hero& heroAfter);

  namespace detail
  {
    // Determines outcome summary and awards experience if applicable.
    // Helper used by Combat::attack and Cast::targeted, do not call directly.
    Summary summaryAndExperience(const Hero& heroBeforeFight, Hero& heroAfterFight, const Monster& monsterAfterFight, bool monsterWasSlowed);
  }
} // namespace Combat
