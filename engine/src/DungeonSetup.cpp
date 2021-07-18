#include "engine/DungeonSetup.hpp"

#include <cassert>

DungeonSetup::DungeonSetup(HeroClass heroClass, HeroRace heroRace, unsigned char mapSize)
  : heroClass(heroClass)
  , heroRace(heroRace)
  , startingEquipment({Potion::HealthPotion, Potion::ManaPotion})
  , mapSize(mapSize)
{
}

DungeonSetup::DungeonSetup(HeroClass heroClass,
                           HeroRace heroRace,
                           std::optional<BlacksmithItem> blacksmithItem,
                           std::optional<AlchemistSeal> alchemistSeal,
                           std::optional<LockerableItem> backpack,
                           std::set<Potion> potions,
                           std::optional<GodOrPactmaker> altar,
                           std::optional<MageModifier> mageModifier,
                           std::optional<BazaarModifier> bazaarModifier,
                           std::optional<ThievesModifier> thievesModifier,
                           unsigned char mapSize)
  : heroClass(heroClass)
  , heroRace(heroRace)
  , altar(altar)
  , mapSize(mapSize)
{
  if (blacksmithItem)
    startingEquipment.insert(*blacksmithItem);
  if (alchemistSeal)
    startingEquipment.insert(*alchemistSeal);
  if (backpack)
  {
    if (auto item = std::get_if<ShopItem>(&*backpack))
      startingEquipment.insert(*item);
    else
      startingEquipment.insert(std::get<BossReward>(*backpack));
  }
  if (!potions.empty())
  {
    assert(potions.size() <= 4);
    startingEquipment.insert(begin(potions), end(potions));
  }
  if (mageModifier)
    modifiers.insert(*mageModifier);
  if (bazaarModifier)
    modifiers.insert(*bazaarModifier);
  if (thievesModifier)
  {
    if (*thievesModifier == ThievesModifier::PatchesTheTeddy)
      startingEquipment.insert(MiscItem::PatchesTheTeddy);
    else
      modifiers.insert(*thievesModifier);
  }
}
