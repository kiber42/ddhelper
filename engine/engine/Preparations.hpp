#pragma once

#include "engine/GodsAndBoons.hpp"
#include "engine/Items.hpp"
#include "engine/Resources.hpp"

#include <cassert>
#include <optional>
#include <set>

struct Preparations
{
  Preparations();

  Preparations(std::optional<BlacksmithItem> blacksmithItem,
               std::optional<AlchemistSeal> alchemistSeal,
               std::optional<ShopItem> backpack,
               std::set<Potion> potions,
               std::optional<GodOrPactmaker> altar,
               std::optional<ResourceModifier> mageModifier,
               std::optional<ResourceModifier> bazaarModifier,
               std::optional<ResourceModifier> thiefModifier);

  int numberOfLargeInventorySlots() const;

  int initialSpellConversionPoints() const;

  std::set<Item> items;
  std::set<ResourceModifier> modifiers;
  std::optional<GodOrPactmaker> altar;
};
