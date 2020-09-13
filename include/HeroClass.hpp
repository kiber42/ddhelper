#pragma once

#include "HeroTraits.hpp"

enum class HeroClass
{
  Fighter,
  Berserker,
  Warlord,
  Wizard,
  Sorcerer,
  Bloodmage,
  Thief,
  Rogue,
  Assassin,
  Priest,
  Monk,
  Paladin,
  Crusader,
  Transmuter,
  Tinker,
  Chemist,
  Vampire,
  HalfDragon,
  Gorgon,
  RatMonarch,
  Goatperson,
  Guard,
};

constexpr const char* to_string(HeroClass theClass)
{
  switch (theClass)
  {
  case HeroClass::Fighter:
    return "Fighter";
  case HeroClass::Berserker:
    return "Berserker";
  case HeroClass::Warlord:
    return "Warlord";
  case HeroClass::Wizard:
    return "Wizard";
  case HeroClass::Sorcerer:
    return "Sorcerer";
  case HeroClass::Bloodmage:
    return "Bloodmage";
  case HeroClass::Thief:
    return "Thief";
  case HeroClass::Rogue:
    return "Rogue";
  case HeroClass::Assassin:
    return "Assassin";
  case HeroClass::Priest:
    return "Priest";
  case HeroClass::Monk:
    return "Monk";
  case HeroClass::Paladin:
    return "Paladin";
  case HeroClass::Crusader:
    return "Crusader";
  case HeroClass::Transmuter:
    return "Transmuter";
  case HeroClass::Tinker:
    return "Tinker";
  case HeroClass::Chemist:
    return "Chemist";
  case HeroClass::Vampire:
    return "Vampire";
  case HeroClass::HalfDragon:
    return "HalfDragon";
  case HeroClass::Gorgon:
    return "Gorgon";
  case HeroClass::RatMonarch:
    return "RatMonarch";
  case HeroClass::Goatperson:
    return "Goatperson";
  case HeroClass::Guard:
    return "Guard";
  }
}
