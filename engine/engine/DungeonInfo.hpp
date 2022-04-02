#pragma once

#include "engine/Boss.hpp"
#include "engine/Monster.hpp"

#include <vector>

enum class Dungeon
{
  HobblersHold,
  DenOfDanger,
  VentureCave,
  WesternJungle,
  EasternTundra,
  NorthernDesert,
  SouthernSwamp,
  Doubledoom,
  GrimmsGrotto,
  RockGarden,
  CursedOasis,
  ShiftingPassages,
  HavendaleBridge,
  TheLabyrinth,
  MagmaMines,
  HexxRuins,
  IckSwamp,
  TheSlimePit,
  BerserkerCamp,
  CreeplightRuins,
  HallsOfSteel,
  MonsterMachine1,
};

constexpr const char* toString(Dungeon dungeon)
{
  switch (dungeon)
  {
  case Dungeon::HobblersHold:
    return "Hobbler's Hold";
  case Dungeon::DenOfDanger:
    return "Den of Danger";
  case Dungeon::VentureCave:
    return "Venture Cave";
  case Dungeon::WesternJungle:
    return "Western Jungle";
  case Dungeon::EasternTundra:
    return "Eastern Tundra";
  case Dungeon::NorthernDesert:
    return "Northern Desert";
  case Dungeon::SouthernSwamp:
    return "Southern Swamp";
  case Dungeon::Doubledoom:
    return "Doubledoom";
  case Dungeon::GrimmsGrotto:
    return "Grimm's Grotto";
  case Dungeon::RockGarden:
    return "Rock Garden";
  case Dungeon::CursedOasis:
    return "Cursed Oasis";
  case Dungeon::ShiftingPassages:
    return "Shifting Passages";
  case Dungeon::HavendaleBridge:
    return "Havendale Bridge";
  case Dungeon::TheLabyrinth:
    return "The Labyrinth";
  case Dungeon::MagmaMines:
    return "Magma Mines";
  case Dungeon::HexxRuins:
    return "Hexx Ruins";
  case Dungeon::IckSwamp:
    return "Ick Swamp";
  case Dungeon::TheSlimePit:
    return "The Slime Pit";
  case Dungeon::BerserkerCamp:
    return "Berserker Camp";
  case Dungeon::CreeplightRuins:
    return "Creeplight Ruins";
  case Dungeon::HallsOfSteel:
    return "Halls of Steel";
  case Dungeon::MonsterMachine1:
    return "Monster Machine 1";
  }
};

constexpr float dungeonMultiplier(Dungeon dungeon)
{
  switch (dungeon)
  {
  case Dungeon::HobblersHold:
    return 0.8f;
  case Dungeon::DenOfDanger:
    return 1.0f;
  case Dungeon::VentureCave:
    return 1.0f;
  case Dungeon::WesternJungle:
    return 1.0f;
  case Dungeon::EasternTundra:
    return 1.0f;
  case Dungeon::NorthernDesert:
    return 1.0f;
  case Dungeon::SouthernSwamp:
    return 1.0f;
  case Dungeon::Doubledoom:
    return 1.1f;
  case Dungeon::GrimmsGrotto:
    return 1.4f;
  case Dungeon::RockGarden:
    return 1.0f;
  case Dungeon::CursedOasis:
    return 1.15f;
  case Dungeon::ShiftingPassages:
    return 1.299f;
  case Dungeon::HavendaleBridge:
    return 1.05f;
  case Dungeon::TheLabyrinth:
    return 1.3f;
  case Dungeon::MagmaMines:
    return 1.3f;
  case Dungeon::HexxRuins:
    return 1.0f;
  case Dungeon::IckSwamp:
    return 1.2f;
  case Dungeon::TheSlimePit:
    return 1.2f;
  case Dungeon::BerserkerCamp:
    return 1.0f;
  case Dungeon::CreeplightRuins:
    return 1.1f;
  case Dungeon::HallsOfSteel:
    return 1.2f;
  case Dungeon::MonsterMachine1:
    return 1.299f;
  }
}

std::vector<Monster> boss(Dungeon dungeon);
std::vector<BossType> boss(Dungeon dungeon, MonsterType type);
