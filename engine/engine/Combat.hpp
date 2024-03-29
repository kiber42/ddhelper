#pragma once

#include "engine/Faith.hpp"
#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Outcome.hpp"

namespace Combat
{
  // Perform melee attack on monster, evaluate effects on all monsters
  Summary attack(Hero&, Monster&, Monsters&, Resources&);

  struct Knockback
  {
    enum class TargetType
    {
      Monster,
      Wall,
      Empty,
      Indestructible
    };

    Knockback(Monster& monster)
      : targetType(TargetType::Monster)
      , monster(&monster)
    {
    }

    Knockback(TargetType targetType)
      : targetType(targetType)
      , monster(nullptr)
    {
    }

    static Knockback intoWall() { return {TargetType::Wall}; }

    TargetType targetType;
    Monster* monster;
  };

  Summary
  attackWithKnockback(Hero& hero, Monster& primary, Monsters& allMonsters, Knockback knockback, Resources& resources);

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
                           Monsters& allMonsters,
                           Resources& resources);

    // Apply debuffs from monster hit.  Piety collection must be activated before (@see Hero::startPietyCollection).
    void applyHitSideEffects(Hero& hero, const Monster& monster);
  } // namespace detail
} // namespace Combat
