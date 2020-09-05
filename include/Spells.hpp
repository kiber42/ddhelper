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
