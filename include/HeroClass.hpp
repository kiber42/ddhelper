#pragma once

#include "HeroTraits.hpp"

#include <stdexcept>
#include <vector>

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
    return "Half Dragon";
  case HeroClass::Gorgon:
    return "Gorgon";
  case HeroClass::RatMonarch:
    return "Rat Monarch";
  case HeroClass::Goatperson:
    return "Goatperson";
  case HeroClass::Guard:
    return "Guard";
  }
}

inline std::vector<HeroTrait> startingTraits(HeroClass theClass)
{
  switch (theClass)
  {
  case HeroClass::Fighter:
    return {HeroTrait::Instincts, HeroTrait::Veteran, HeroTrait::PitDog};
  case HeroClass::Berserker:
    return {HeroTrait::Bloodlust, HeroTrait::Mageslay, HeroTrait::Spellkill};
  case HeroClass::Warlord:
    // return {HeroTrait::Courageous, HeroTrait::Determined, HeroTrait::Defiant};
  case HeroClass::Wizard:
    // return {HeroTrait::MagicSense, HeroTrait::MagicAffinity, HeroTrait::MagicAttunement};
  case HeroClass::Sorcerer:
    // return {HeroTrait::EssenceTransit, HeroTrait::ArcaneKnowledge, HeroTrait::ManaShield};
  case HeroClass::Bloodmage:
    // return {HeroTrait::PowerHungry, HeroTrait::Sanguine, HeroTrait::Insane};
  case HeroClass::Thief:
    // return {HeroTrait::Stabber, HeroTrait::Hoarder, HeroTrait::Survivor};
  case HeroClass::Rogue:
    // return {HeroTrait::Dangerous, HeroTrait::Dexterous, HeroTrait::Evasive};
  case HeroClass::Assassin:
    // return {HeroTrait::PoisonedBlade, HeroTrait::LightFoot, HeroTrait::SwiftHand};
  case HeroClass::Priest:
    // return {HeroTrait::GoodHealth, HeroTrait::GoodDrink, HeroTrait::GoodGolly};
  case HeroClass::Monk:
    // return {HeroTrait::HandToHand, HeroTrait::Discipline, HeroTrait::DiamondBody};
  case HeroClass::Paladin:
    // return {HeroTrait::HolyWork, HeroTrait::HolyHands, HeroTrait::HolyShield};
  case HeroClass::Crusader:
    // return {HeroTrait::Scars, HeroTrait::Momentum, HeroTrait::Martyr};
  case HeroClass::Transmuter:
    // return {HeroTrait::InnerFocus, HeroTrait::DungeonLore, HeroTrait::SpiritSword};
  case HeroClass::Tinker:
    // return {HeroTrait::Merchant, HeroTrait::Negotiator, HeroTrait::Macguyver};
  case HeroClass::Chemist:
    // return {HeroTrait::Additives, HeroTrait::Preservatives, HeroTrait::Colourants};
  case HeroClass::Vampire:
    // return {HeroTrait::Undead, HeroTrait::Damned, HeroTrait::EternalThirst};
  case HeroClass::HalfDragon:
    // return {HeroTrait::DragonBreath, HeroTrait::DragonTail, HeroTrait::DragonStature};
  case HeroClass::Gorgon:
    // return {HeroTrait::AzureBody, HeroTrait::SapphireLocks, HeroTrait::AmethystStare};
  case HeroClass::RatMonarch:
    // return {HeroTrait::RegalHygiene, HeroTrait::RegalPerks, HeroTrait::RegalSize};
  case HeroClass::Goatperson:
    // return {HeroTrait::Scapegoat, HeroTrait::Prototype, HeroTrait::Herbivore};
    break;
  case HeroClass::Guard:
    // Tutorial class without traits, also used for unit tests
    return {};
  }
  throw std::runtime_error("not implemented");
}
