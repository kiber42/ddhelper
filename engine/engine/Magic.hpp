#pragma once

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Outcome.hpp"
#include "engine/Spells.hpp"

namespace Magic
{
  // Determine whether spell can currently be cast
  bool isPossible(const Hero& hero, Spell spell, const Resources& resources);
  bool isPossible(const Hero& hero, const Monster& monster, Spell spell, const Resources& resources);

  // Cast spell that does not target a monster
  void cast(Hero& hero, Spell spell, Monsters& allMonsters, Resources& resources);

  // Cast spell on monster, evaluate effect on remaining monsters
  Summary cast(Hero& hero, Monster& monster, Spell spell, Monsters& allMonsters, Resources& resources);

  // Spells that need to target a monster
  constexpr bool needsMonster(Spell spell)
  {
    return spell == Spell::Apheelsik || spell == Spell::Burndayraz || spell == Spell::Pisorf ||
           spell == Spell::Weytwut || spell == Spell::Wonafyt;
  }

  // Spells that behave differently when targeted at a monster
  constexpr bool monsterIsOptional(Spell spell) { return spell == Spell::Imawal || spell == Spell::Lemmisi; }

  // Spells that might not succeed (chance depends on monster's magic resistance)
  constexpr bool canBeResisted(Spell spell)
  {
    return spell == Spell::Imawal || spell == Spell::Weytwut || spell == Spell::Wonafyt;
  }

  unsigned spellCosts(Spell spell, const Hero& hero);

  unsigned healthCostsBludtupowa(const Hero& hero);
} // namespace Magic
