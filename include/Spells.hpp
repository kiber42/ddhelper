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
  Wonafyt
};

namespace Cast
{
  // Determine whether spell can currently be cast
  bool isPossible(const Hero& hero, Spell spell);
  bool isPossible(const Hero& hero, const Monster& monster, Spell spell);
  // Cast spell that does not target a monster
  Hero untargeted(Hero hero, Spell spell);
  // Predict outcome of casting spell on monster
  Outcome predictOutcome(const Hero& hero, const Monster& monster, Spell spell);
} // namespace Cast
