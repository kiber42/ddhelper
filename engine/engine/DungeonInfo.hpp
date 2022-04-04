#pragma once

#include "engine/ConstExprMap.hpp"
#include "engine/StrongTypes.hpp"

#include <string_view>
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
  Last = MonsterMachine1,
};

struct DungeonInfo
{
  std::string_view name;
  DungeonMultiplier multiplier;
};

using namespace std::string_view_literals;
static constexpr Map<Dungeon, DungeonInfo, 22> dungeons{
    std::make_pair(Dungeon::HobblersHold, DungeonInfo{"Hobbler's Hold"sv, DungeonMultiplier{0.8f}}),
    {Dungeon::DenOfDanger, {"Den of Danger"sv, DungeonMultiplier{1}}},
    {Dungeon::VentureCave, {"Venture Cave"sv, DungeonMultiplier{1}}},
    {Dungeon::WesternJungle, {"Western Jungle"sv, DungeonMultiplier{1}}},
    {Dungeon::EasternTundra, {"Eastern Tundra"sv, DungeonMultiplier{1}}},
    {Dungeon::NorthernDesert, {"Northern Desert"sv, DungeonMultiplier{1}}},
    {Dungeon::SouthernSwamp, {"Southern Swamp"sv, DungeonMultiplier{1}}},
    {Dungeon::Doubledoom, {"Doubledoom"sv, DungeonMultiplier{1.1f}}},
    {Dungeon::GrimmsGrotto, {"Grimm's Grotto"sv, DungeonMultiplier{1.4f}}},
    {Dungeon::RockGarden, {"Rock Garden"sv, DungeonMultiplier{1}}},
    {Dungeon::CursedOasis, {"Cursed Oasis"sv, DungeonMultiplier{1.15f}}},
    {Dungeon::ShiftingPassages, {"Shifting Passages"sv, DungeonMultiplier{1.299f}}},
    {Dungeon::HavendaleBridge, {"Havendale Bridge"sv, DungeonMultiplier{1.05f}}},
    {Dungeon::TheLabyrinth, {"The Labyrinth"sv, DungeonMultiplier{1.3f}}},
    {Dungeon::MagmaMines, {"Magma Mines"sv, DungeonMultiplier{1.3f}}},
    {Dungeon::HexxRuins, {"Hexx Ruins"sv, DungeonMultiplier{1}}},
    {Dungeon::IckSwamp, {"Ick Swamp"sv, DungeonMultiplier{1.2f}}},
    {Dungeon::TheSlimePit, {"The Slime Pit"sv, DungeonMultiplier{1.2f}}},
    {Dungeon::BerserkerCamp, {"Berserker Camp"sv, DungeonMultiplier{1}}},
    {Dungeon::CreeplightRuins, {"Creeplight Ruins"sv, DungeonMultiplier{1.1f}}},
    {Dungeon::HallsOfSteel, {"Halls of Steel"sv, DungeonMultiplier{1.2f}}},
    {Dungeon::MonsterMachine1, {"Monster Machine 1"sv, DungeonMultiplier{1.3f}}},
};

constexpr const char* toString(Dungeon dungeon)
{
  return dungeons.at(dungeon).name.data();
};

constexpr DungeonMultiplier dungeonMultiplier(Dungeon dungeon)
{
  return dungeons.at(dungeon).multiplier;
}
