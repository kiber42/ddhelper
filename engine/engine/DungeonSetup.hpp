#pragma once

#include "engine/GodsAndBoons.hpp"
#include "engine/HeroClass.hpp"
#include "engine/Items.hpp"

#include <cassert>
#include <optional>
#include <set>
#include <variant>

static const unsigned char DefaultMapSize = 20;

enum class MageModifier
{
  ExtraAttackBoosters,
  ExtraManaBoosters,
  ExtraHealthBoosters,
  FlameMagnet,
  FewerGlyphs,
  ExtraGlyph,
};

enum class BazaarModifier
{
  QuestItems, // TODO
  EliteItems, // TODO
  Apothecary,
  ExtraAltar,
};

enum class ThievesModifier
{
  BlackMarket,
  SmugglerDen, // TODO
  PatchesTheTeddy,
};

using DungeonModifier = std::variant<MageModifier, BazaarModifier, ThievesModifier>;

/** TODO: Preparation penalties:
 *  Dracul: 25% of enemies are bloodless
 *  Taurog: all enemies are cowardly
 */

struct DungeonSetup
{
  explicit DungeonSetup(HeroClass = HeroClass::Guard, HeroRace = HeroRace::Human, unsigned char mapSize = DefaultMapSize);

  DungeonSetup(HeroClass heroClass,
               HeroRace heroRace,
               std::optional<BlacksmithItem> blacksmithItem,
               std::optional<AlchemistSeal> alchemistSeal,
               std::optional<ShopItem> backpack,
               std::set<Potion> potions,
               std::optional<GodOrPactmaker> altar,
               std::optional<MageModifier> mageModifier,
               std::optional<BazaarModifier> bazaarModifier,
               std::optional<ThievesModifier> thiefModifier,
               unsigned char mapSize = DefaultMapSize);

  HeroClass heroClass;
  HeroRace heroRace;
  std::set<Item> startingEquipment;
  std::set<DungeonModifier> modifiers;
  std::optional<GodOrPactmaker> altar;
  unsigned char mapSize;
};
