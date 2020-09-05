#pragma once

// Traits are usually available from the beginning (except for god boons)
// and cannot be removed
enum class HeroTrait
{
  Additives,       // Chemist
  Damned,          // Vampire
  Defiant,         // Warlord
  Discipline,      // Monk
  HolyHands,       // Paladin
  MagicAttunement, // Wizard
  // Boons
  BloodCurse,      // Dracul
  Flames,          // Mystera Annur
  Humility,        // Glowing Guardian
};

constexpr const char* toString(HeroTrait trait)
{
  switch (trait)
  {
  case HeroTrait::Additives:
    return "Additives";
  case HeroTrait::Damned:
    return "Damned";
  case HeroTrait::Defiant:
    return "Defiant";
  case HeroTrait::Discipline:
    return "Discipline";
  case HeroTrait::HolyHands:
    return "Holy Hands";
  case HeroTrait::MagicAttunement:
    return "Magic Attunement";

  case HeroTrait::BloodCurse:
    return "Blood Curse";
  case HeroTrait::Flames:
    return "Flames";
  case HeroTrait::Humility:
    return "Humility";
  }
}
