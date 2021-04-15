#pragma once

#include <array>
#include <variant>

enum class God
{
  BinlorIronshield,
  Dracul,
  TheEarthmother,
  GlowingGuardian,
  JehoraJeheyu,
  MysteraAnnur,
  Taurog,
  TikkiTooki,
  Last = TikkiTooki
};

enum class Pactmaker
{
  ThePactmaker
};

enum class Boon
{
  // Binlor Ironshield
  StoneSoup,
  StoneSkin,
  StoneForm,
  StoneFist,
  StoneHeart,

  // Dracul
  BloodCurse,
  BloodTithe,
  BloodHunger,
  BloodShield,
  BloodSwell,

  // The Earthmother
  Plantation,
  Clearance,
  Greenblood,
  Entanglement,
  VineForm,

  // Glowing Guardian
  Humility,
  Absolution,
  Cleansing,
  Protection,
  Enlightenment,

  // Jehora
  Petition,
  LastChance,
  BoostHealth,
  BoostMana,
  ChaosAvatar,

  // Mystera Annur
  Magic,
  Refreshment,
  Flames,
  Weakening,
  MysticBalance,

  // Taurog
  TaurogsBlade,
  TaurogsShield,
  TaurogsHelm,
  TaurogsArmour,
  UnstoppableFury,

  // Tikki Tooki
  Tribute,
  TikkisEdge,
  Dodging,
  Poison,
  Reflexes,

  Last = Reflexes
};

enum class Pact
{
  ScholarsPact,
  WarriorsPact,
  AlchemistsPact,
  BodyPact,
  SpiritPact,
  Consensus,

  LastNoConsensus = SpiritPact,
  LastWithConsensus = Consensus
};

using GodOrPactmaker = std::variant<God, Pactmaker>;
using BoonOrPact = std::variant<Boon, Pact>;

constexpr const char* toString(God god)
{
  switch (god)
  {
  case God::BinlorIronshield:
    return "Binlor Ironshield";
  case God::Dracul:
    return "Dracul";
  case God::TheEarthmother:
    return "The Earthmother";
  case God::GlowingGuardian:
    return "Glowing Guardian";
  case God::JehoraJeheyu:
    return "Jehora Jeheyu";
  case God::MysteraAnnur:
    return "Mystera Annur";
  case God::Taurog:
    return "Taurog";
  case God::TikkiTooki:
    return "Tikki Tooki";
  }
}

constexpr const char* toString(Pactmaker)
{
  return "The Pactmaker";
}

constexpr const char* toString(GodOrPactmaker godOrPactmaker)
{
  if (auto god = std::get_if<God>(&godOrPactmaker))
    return toString(*god);
  return toString(std::get<Pactmaker>(godOrPactmaker));
}

constexpr const char* toString(Boon boon)
{
  switch (boon)
  {
  case Boon::StoneSoup:
    return "Stone Soup";
  case Boon::StoneSkin:
    return "Stone Skin";
  case Boon::StoneForm:
    return "Stone Form";
  case Boon::StoneFist:
    return "Stone Fist";
  case Boon::StoneHeart:
    return "Stone Heart";

  case Boon::BloodCurse:
    return "Blood Curse";
  case Boon::BloodTithe:
    return "Blood Tithe";
  case Boon::BloodHunger:
    return "Blood Hunger";
  case Boon::BloodShield:
    return "Blood Shield";
  case Boon::BloodSwell:
    return "Blood Swell";

  case Boon::Plantation:
    return "Plantation";
  case Boon::Clearance:
    return "Clearance";
  case Boon::Greenblood:
    return "Greenblood";
  case Boon::Entanglement:
    return "Entanglement";
  case Boon::VineForm:
    return "Vine Form";

  case Boon::Humility:
    return "Humility";
  case Boon::Absolution:
    return "Absolution";
  case Boon::Cleansing:
    return "Cleansing";
  case Boon::Protection:
    return "Protection";
  case Boon::Enlightenment:
    return "Enlightenment";

  case Boon::Petition:
    return "Petition";
  case Boon::LastChance:
    return "Last Chance";
  case Boon::BoostHealth:
    return "Boost Health";
  case Boon::BoostMana:
    return "Boost Mana";
  case Boon::ChaosAvatar:
    return "Chaos Avatar";

  case Boon::Magic:
    return "Magic";
  case Boon::Refreshment:
    return "Refreshment";
  case Boon::Flames:
    return "Flames";
  case Boon::Weakening:
    return "Weakening";
  case Boon::MysticBalance:
    return "Mystic Balance";

  case Boon::TaurogsBlade:
    return "Taurog's Blade";
  case Boon::TaurogsShield:
    return "Taurog's Shield";
  case Boon::TaurogsHelm:
    return "Taurog's Helm";
  case Boon::TaurogsArmour:
    return "Taurog's Armour";
  case Boon::UnstoppableFury:
    return "Unstoppable Fury";

  case Boon::Tribute:
    return "Tribute";
  case Boon::TikkisEdge:
    return "Tikki's Edge";
  case Boon::Dodging:
    return "Dodging";
  case Boon::Poison:
    return "Poison";
  case Boon::Reflexes:
    return "Reflexes";
  }
}

constexpr const char* toString(Pact pact)
{
  switch (pact)
  {
  case Pact::ScholarsPact:
    return "Scholars Pact";
  case Pact::WarriorsPact:
    return "Warriors Pact";
  case Pact::AlchemistsPact:
    return "Alchemists Pact";
  case Pact::BodyPact:
    return "Body Pact";
  case Pact::SpiritPact:
    return "Spirit Pact";
  case Pact::Consensus:
    return "Consensus";
  }
};

constexpr bool allowRepeatedUse(Boon boon)
{
  return boon == Boon::StoneSkin || boon == Boon::StoneHeart || boon == Boon::BloodTithe || boon == Boon::BloodHunger ||
         boon == Boon::BloodSwell || boon == Boon::Plantation || boon == Boon::Clearance || boon == Boon::Greenblood ||
         boon == Boon::Entanglement || boon == Boon::VineForm || boon == Boon::Absolution || boon == Boon::Cleansing ||
         boon == Boon::Protection || boon == Boon::BoostHealth || boon == Boon::BoostMana || boon == Boon::Magic ||
         boon == Boon::Weakening || boon == Boon::UnstoppableFury || boon == Boon::Tribute ||
         boon == Boon::TikkisEdge || boon == Boon::Poison || boon == Boon::Reflexes;
}

constexpr God deity(Boon boon)
{
  switch (boon)
  {
  case Boon::StoneSoup:
  case Boon::StoneSkin:
  case Boon::StoneForm:
  case Boon::StoneFist:
  case Boon::StoneHeart:
    return God::BinlorIronshield;

  case Boon::BloodCurse:
  case Boon::BloodTithe:
  case Boon::BloodHunger:
  case Boon::BloodShield:
  case Boon::BloodSwell:
    return God::Dracul;

  case Boon::Plantation:
  case Boon::Clearance:
  case Boon::Greenblood:
  case Boon::Entanglement:
  case Boon::VineForm:
    return God::TheEarthmother;

  case Boon::Humility:
  case Boon::Absolution:
  case Boon::Cleansing:
  case Boon::Protection:
  case Boon::Enlightenment:
    return God::GlowingGuardian;

  case Boon::Petition:
  case Boon::LastChance:
  case Boon::BoostHealth:
  case Boon::BoostMana:
  case Boon::ChaosAvatar:
    return God::JehoraJeheyu;

  case Boon::Magic:
  case Boon::Refreshment:
  case Boon::Flames:
  case Boon::Weakening:
  case Boon::MysticBalance:
    return God::MysteraAnnur;

  case Boon::TaurogsBlade:
  case Boon::TaurogsShield:
  case Boon::TaurogsHelm:
  case Boon::TaurogsArmour:
  case Boon::UnstoppableFury:
    return God::Taurog;

  case Boon::Tribute:
  case Boon::TikkisEdge:
  case Boon::Dodging:
  case Boon::Poison:
  case Boon::Reflexes:
    return God::TikkiTooki;
  }
}

constexpr std::array<Boon, 5> offeredBoons(God god)
{
  switch (god)
  {
  case God::BinlorIronshield:
    return {Boon::StoneSoup, Boon::StoneSkin, Boon::StoneForm, Boon::StoneFist, Boon::StoneHeart};
  case God::Dracul:
    return {Boon::BloodCurse, Boon::BloodTithe, Boon::BloodHunger, Boon::BloodShield, Boon::BloodSwell};
  case God::TheEarthmother:
    return {Boon::Plantation, Boon::Clearance, Boon::Greenblood, Boon::Entanglement, Boon::VineForm};
  case God::GlowingGuardian:
    return {Boon::Humility, Boon::Absolution, Boon::Cleansing, Boon::Protection, Boon::Enlightenment};
  case God::JehoraJeheyu:
    return {Boon::Petition, Boon::LastChance, Boon::BoostHealth, Boon::BoostMana, Boon::ChaosAvatar};
  case God::MysteraAnnur:
    return {Boon::Magic, Boon::Refreshment, Boon::Flames, Boon::Weakening, Boon::MysticBalance};
  case God::Taurog:
    return {Boon::TaurogsBlade, Boon::TaurogsShield, Boon::TaurogsHelm, Boon::TaurogsArmour, Boon::UnstoppableFury};
  case God::TikkiTooki:
    return {Boon::Tribute, Boon::TikkisEdge, Boon::Dodging, Boon::Poison, Boon::Reflexes};
  }
}
