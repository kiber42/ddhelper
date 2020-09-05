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
  bool isPossible(const Hero& hero, const Monster& monster, Spell spell);
  Outcome predictOutcome(const Hero& hero, const Monster& monster, Spell spell);
}
