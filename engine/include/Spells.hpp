#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

enum class Spell
{
  Apheelsik,
  Bludtupowa,
  Burndayraz,
  Bysseps,
  Cydstepp,
  Endiswal,
  Getindare,
  Halpmeh,
  Imawal,
  Lemmisi,
  Pisorf,
  Weytwut,
  Wonafyt,
  Last = Wonafyt
};

constexpr const char* toString(Spell spell)
{
  switch (spell)
  {
  case Spell::Apheelsik:
    return "Apheelsik";
  case Spell::Bludtupowa:
    return "Bludtupowa";
  case Spell::Burndayraz:
    return "Burndayraz";
  case Spell::Bysseps:
    return "Bysseps";
  case Spell::Cydstepp:
    return "Cydstepp";
  case Spell::Endiswal:
    return "Endiswal";
  case Spell::Getindare:
    return "Getindare";
  case Spell::Halpmeh:
    return "Halpmeh";
  case Spell::Imawal:
    return "Imawal";
  case Spell::Lemmisi:
    return "Lemmisi";
  case Spell::Pisorf:
    return "Pisorf";
  case Spell::Weytwut:
    return "Weytwut";
  case Spell::Wonafyt:
    return "Wonafyt";
  }
}

namespace Magic
{
  // Determine whether spell can currently be cast
  bool isPossible(const Hero& hero, Spell spell);
  bool isPossible(const Hero& hero, const Monster& monster, Spell spell);

  // Cast spell that does not target a monster
  void cast(Hero& hero, Spell spell);

  // Cast spell on monster, evaluate effect on remaining monsters
  Summary cast(Hero& hero, Monster& monster, Monsters& monsters, Spell spell);

  // Spells that need to target a monster
  constexpr bool needsMonster(Spell spell)
  {
    return spell == Spell::Apheelsik || spell == Spell::Burndayraz || spell == Spell::Pisorf ||
           spell == Spell::Weytwut || spell == Spell::Wonafyt;
  }

  // Spells that behave differently when targeted at a monster
  constexpr bool monsterIsOptional(Spell spell)
  {
    return spell == Spell::Imawal || spell == Spell::Lemmisi;
  }

  // Spells that might not succeed (chance depends on monster's magic resistance)
  constexpr bool canBeResisted(Spell spell)
  {
    return spell == Spell::Imawal || spell == Spell::Weytwut || spell == Spell::Wonafyt;
  }

  int spellCosts(Spell spell, const Hero& hero);
} // namespace Magic
