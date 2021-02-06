#pragma once

// Traits are available from the beginning and cannot be removed
enum class HeroTrait
{
  Additives,
  AmethystStare,
  ArcaneKnowledge,
  AzureBody,
  Bloodlust,
  Colourants,
  Courageous,
  Damned,
  Dangerous,
  Defiant,
  Determined,
  Dexterous,
  DiamondBody,
  Discipline,
  DragonBreath,
  DragonStature,
  DragonTail,
  DungeonLore,
  EssenceTransit,
  EternalThirst,
  Evasive,
  GoodDrink,
  GoodGolly,
  GoodHealth,
  HandToHand,
  Herbivore,
  Hoarder, // not implemented: +33% more items on dungeon map
  HolyHands,
  HolyShield,
  HolyWork,
  InnerFocus,
  Insane,    // partially implemented, missing: drinking blood restores 1 mana
  Instincts, // not implemented: monsters of equal or lower level always have their location revealed
  LightFoot, // not implemented: exploring the area around a monster grants first strike
  Macguyver, // partially implemented, missing: can sense dungeon stairs
  Mageslay,
  MagicAffinity,
  MagicAttunement,
  MagicSense, // not implemented: can see locations of all glyphs from level start, all glyphs are small items
  ManaShield,
  Martyr,   // not implemented: +1 altar in dungeon
  Merchant, // not implemented: +2 shops in dungeon
  Momentum,
  Negotiator,
  PitDog,
  PoisonedBlade,
  PowerHungry,
  Preservatives,
  Prototype,
  RegalHygiene,
  RegalPerks,
  RegalSize,
  Sanguine,
  SapphireLocks,
  Scapegoat,
  Scars,
  Spellkill,
  SpiritSword,
  Stabber,
  Survivor,
  SwiftHand,
  Undead,
  Veteran,
};

constexpr const char* toString(HeroTrait trait)
{
  switch (trait)
  {
  case HeroTrait::Additives:
    return "Additives";
  case HeroTrait::AmethystStare:
    return "Amethyst Stare";
  case HeroTrait::ArcaneKnowledge:
    return "Arcane Knowledge";
  case HeroTrait::AzureBody:
    return "Azure Body";
  case HeroTrait::Bloodlust:
    return "Bloodlust";
  case HeroTrait::Colourants:
    return "Colourants";
  case HeroTrait::Courageous:
    return "Courageous";
  case HeroTrait::Damned:
    return "Damned";
  case HeroTrait::Dangerous:
    return "Dangerous";
  case HeroTrait::Defiant:
    return "Defiant";
  case HeroTrait::Determined:
    return "Determined";
  case HeroTrait::Dexterous:
    return "Dexterous";
  case HeroTrait::DiamondBody:
    return "Diamond Body";
  case HeroTrait::Discipline:
    return "Discipline";
  case HeroTrait::DragonBreath:
    return "Dragon Breath";
  case HeroTrait::DragonStature:
    return "Dragon Stature";
  case HeroTrait::DragonTail:
    return "Dragon Tail";
  case HeroTrait::DungeonLore:
    return "Dungeon Lore";
  case HeroTrait::EssenceTransit:
    return "Essence Transit";
  case HeroTrait::EternalThirst:
    return "Eternal Thirst";
  case HeroTrait::Evasive:
    return "Evasive";
  case HeroTrait::GoodDrink:
    return "Good Drink";
  case HeroTrait::GoodGolly:
    return "Good Golly";
  case HeroTrait::GoodHealth:
    return "Good Health";
  case HeroTrait::HandToHand:
    return "Hand To Hand";
  case HeroTrait::Herbivore:
    return "Herbivore";
  case HeroTrait::Hoarder:
    return "Hoarder";
  case HeroTrait::HolyHands:
    return "Holy Hands";
  case HeroTrait::HolyShield:
    return "Holy Shield";
  case HeroTrait::HolyWork:
    return "Holy Work";
  case HeroTrait::InnerFocus:
    return "Inner Focus";
  case HeroTrait::Insane:
    return "Insane";
  case HeroTrait::Instincts:
    return "Instincts";
  case HeroTrait::LightFoot:
    return "Light Foot";
  case HeroTrait::Macguyver:
    return "Macguyver";
  case HeroTrait::Mageslay:
    return "Mageslay";
  case HeroTrait::MagicAffinity:
    return "Magic Affinity";
  case HeroTrait::MagicAttunement:
    return "Magic Attunement";
  case HeroTrait::MagicSense:
    return "Magic Sense";
  case HeroTrait::ManaShield:
    return "Mana Shield";
  case HeroTrait::Martyr:
    return "Martyr";
  case HeroTrait::Merchant:
    return "Merchant";
  case HeroTrait::Momentum:
    return "Momentum";
  case HeroTrait::Negotiator:
    return "Negotiator";
  case HeroTrait::PitDog:
    return "Pit Dog";
  case HeroTrait::PoisonedBlade:
    return "Poisoned Blade";
  case HeroTrait::PowerHungry:
    return "Power-Hungry";
  case HeroTrait::Preservatives:
    return "Preservatives";
  case HeroTrait::Prototype:
    return "Prototype";
  case HeroTrait::RegalHygiene:
    return "Regal Hygiene";
  case HeroTrait::RegalPerks:
    return "Regal Perks";
  case HeroTrait::RegalSize:
    return "Regal Size";
  case HeroTrait::Sanguine:
    return "Sanguine";
  case HeroTrait::SapphireLocks:
    return "Sapphire Locks";
  case HeroTrait::Scapegoat:
    return "Scapegoat";
  case HeroTrait::Scars:
    return "Scars";
  case HeroTrait::Spellkill:
    return "Spellkill";
  case HeroTrait::SpiritSword:
    return "Spirit Sword";
  case HeroTrait::Stabber:
    return "Stabber";
  case HeroTrait::Survivor:
    return "Survivor";
  case HeroTrait::SwiftHand:
    return "Swift Hand";
  case HeroTrait::Undead:
    return "Undead";
  case HeroTrait::Veteran:
    return "Veteran";
  }
}
