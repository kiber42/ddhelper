#include "engine/Preparations.hpp"

#include <cassert>

Preparations::Preparations()
  : items({Potion::HealthPotion, Potion::ManaPotion})
  , modifiers()
  , altar()
{
}

Preparations::Preparations(std::optional<BlacksmithItem> blacksmithItem,
                           std::optional<AlchemistSeal> alchemistSeal,
                           std::optional<ShopItem> backpack,
                           std::set<Potion> potions,
                           std::optional<GodOrPactmaker> altar,
                           std::optional<ResourceModifier> mageModifier,
                           std::optional<ResourceModifier> bazaarModifier,
                           std::optional<ResourceModifier> thiefModifier)
  : altar(altar)
{
  if (blacksmithItem)
    items.insert(*blacksmithItem);
  if (alchemistSeal)
    items.insert(*alchemistSeal);
  if (backpack)
    items.insert(*backpack);
  if (!potions.empty())
  {
    assert(potions.size() <= 4);
    items.insert(begin(potions), end(potions));
  }
  if (mageModifier)
  {
    assert(static_cast<int>(*mageModifier) <= static_cast<int>(ResourceModifier::ExtraGlyph));
    modifiers.insert(*mageModifier);
  }
  if (bazaarModifier)
  {
    [[maybe_unused]] const int modifierIndex = static_cast<int>(*bazaarModifier);
    assert(modifierIndex >= static_cast<int>(ResourceModifier::QuestItems));
    assert(modifierIndex <= static_cast<int>(ResourceModifier::ExtraAltar));
    modifiers.insert(*bazaarModifier);
  }
  if (thiefModifier)
  {
    [[maybe_unused]] const int modifierIndex = static_cast<int>(*thiefModifier);
    assert(modifierIndex >= static_cast<int>(ResourceModifier::BlackMarket));
    assert(modifierIndex <= static_cast<int>(ResourceModifier::PatchesTheTeddy));
    modifiers.insert(*thiefModifier);
  }
  /*
  if (hero)
  {
    if (hero->hasTrait(HeroTrait::Hoarder))
      modifiers.insert(ResourceModifier::Hoarder);
    if (hero->hasTrait(HeroTrait::Martyr))
      modifiers.insert(ResourceModifier::Martyr);
    if (hero->hasTrait(HeroTrait::Merchant))
      modifiers.insert(ResourceModifier::Merchant);
  }
  */
}

int Preparations::numberOfLargeInventorySlots() const
{
  return altar == GodOrPactmaker{God::JehoraJeheyu} ? 5 : 6;
}

int Preparations::initialSpellConversionPoints() const
{
  const bool extraGlyph = std::find(begin(modifiers), end(modifiers), ResourceModifier::ExtraGlyph) != end(modifiers);
  const bool fewerGlyphs = std::find(begin(modifiers), end(modifiers), ResourceModifier::FewerGlyphs) != end(modifiers);
  return 100 + 50 * fewerGlyphs - 20 * extraGlyph;
}
