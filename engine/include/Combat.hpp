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
    // Optionally, evaluates burn stack damage on other monsters.
    Summary finalizeAttack(Hero& hero,
                           const Monster& monster,
                           bool monsterWasSlowed,
                           bool monsterWasBurning,
                           bool triggerBurndown,
                           Monsters& allMonsters);
  } // namespace detail
} // namespace Combat
