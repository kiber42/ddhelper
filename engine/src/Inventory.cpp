#include "engine/Inventory.hpp"

#include "engine/Items.hpp"
#include "engine/Magic.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>

namespace
{
  constexpr int initialSpellConversionPoints(bool hasExtraGlyph, bool hasFewerGlyphs)
  {
    int conversionPoints = 100;
    if (hasExtraGlyph)
      conversionPoints -= 20;
    if (hasFewerGlyphs)
      conversionPoints += 50;
    return conversionPoints;
  }

  constexpr unsigned LargeItemSize = 5;
  constexpr auto FoodItem = Item{MiscItem::Food};
  constexpr auto FoodItemOrSpell = ItemOrSpell{MiscItem::Food};
} // namespace

Inventory::Inventory(const DungeonSetup& setup)
  : numSlots(setup.altar == GodOrPactmaker{God::JehoraJeheyu} ? 5 : 6)
  , spellConversionPoints(initialSpellConversionPoints(setup.modifiers.count(MageModifier::ExtraGlyph),
                                                       setup.modifiers.count(MageModifier::FewerGlyphs)))
  , spellsSmall(hasStartingTrait(setup.heroClass, HeroTrait::MagicSense))
  , allItemsLarge(hasStartingTrait(setup.heroClass, HeroTrait::RegalSize))
  , negotiator(hasStartingTrait(setup.heroClass, HeroTrait::Negotiator))
{
  bool allItemsFit = true;

  if (hasStartingTrait(setup.heroClass, HeroTrait::Herbivore))
  {
    allItemsFit &= add(MiscItem::Food);
    numFood = 90;
  }

  for (const auto& item : setup.startingEquipment)
  {
    const auto potion = std::get_if<Potion>(&item);
    const bool isFreePotion = potion && (*potion == Potion::HealthPotion || *potion == Potion::ManaPotion);
    allItemsFit &= isFreePotion ? addFree(item) : add(item);
  }

  if (hasStartingTrait(setup.heroClass, HeroTrait::Macguyver))
  {
    allItemsFit &= addFree(AlchemistSeal::CompressionSeal) & addFree(AlchemistSeal::CompressionSeal) &
                   addFree(AlchemistSeal::TransmutationSeal) & addFree(AlchemistSeal::TransmutationSeal) &
                   addFree(AlchemistSeal::TranslocationSeal);
  }

  if (hasStartingTrait(setup.heroClass, HeroTrait::Defiant))
    allItemsFit &= add(Spell::Cydstepp);
  if (hasStartingTrait(setup.heroClass, HeroTrait::MagicAttunement) || setup.modifiers.count(MageModifier::FlameMagnet))
    allItemsFit &= add(Spell::Burndayraz);
  if (hasStartingTrait(setup.heroClass, HeroTrait::Insane))
    allItemsFit &= add(Spell::Bludtupowa);
  if (hasStartingTrait(setup.heroClass, HeroTrait::PoisonedBlade))
    allItemsFit &= add(Spell::Apheelsik);
  if (hasStartingTrait(setup.heroClass, HeroTrait::HolyHands))
    allItemsFit &= add(Spell::Halpmeh);
  if (hasStartingTrait(setup.heroClass, HeroTrait::DungeonLore))
    allItemsFit &= add(Spell::Lemmisi);
  if (hasStartingTrait(setup.heroClass, HeroTrait::SapphireLocks))
    allItemsFit &= add(Spell::Endiswal);

  if (!allItemsFit)
    std::cerr << "Not all starting items could be added to inventory." << std::endl;
}

bool Inventory::add(ItemOrSpell itemOrSpell)
{
  if (itemOrSpell == ItemOrSpell{MiscItem::FoodStack})
    return addFood(9);
  if (!hasRoomFor(itemOrSpell))
    return false;
  if (itemOrSpell == FoodItemOrSpell)
  {
    // Allow at most one food item in inventory
    ++numFood;
    if (numFood > 1)
      return true;
  }
  const auto [smallItem, price, conversionPoints] = [&, item = std::get_if<Item>(&itemOrSpell)] {
    if (item)
      return std::tuple{isSmall(*item) && !allItemsLarge, buyingPrice(*item), initialConversionPoints(*item)};
    else
      return std::tuple{spellsSmall && !allItemsLarge, 0, spellConversionPoints};
  }();
  entries.emplace_back(Entry{itemOrSpell, smallItem, price, conversionPoints});
  return true;
}

bool Inventory::addFree(Item item)
{
  if (!hasRoomFor(item))
    return false;
  if (item == FoodItem)
  {
    // Allow at most one food item in inventory
    ++numFood;
    if (numFood > 1)
      return true;
  }
  entries.emplace_back(Entry{ItemOrSpell{item}, isSmall(item) && !allItemsLarge, 0, initialConversionPoints(item)});
  return true;
}

bool Inventory::addFree(Spell spell)
{
  if (!hasRoomFor(spell))
    return false;
  entries.emplace_back(Entry{spell, spellsSmall && !allItemsLarge, 0, 0});
  return true;
}

bool Inventory::has(ItemOrSpell itemOrSpell) const
{
  return std::find_if(begin(entries), end(entries),
                      [itemOrSpell](auto& entry) { return entry.itemOrSpell == itemOrSpell; }) != end(entries);
}

bool Inventory::canConvert(ItemOrSpell itemOrSpell) const
{
  return getConversionPoints(itemOrSpell).has_value();
}

std::optional<unsigned> Inventory::getConversionPoints(ItemOrSpell itemOrSpell) const
{
  const auto it = std::find_if(begin(entries), end(entries), [itemOrSpell](auto& entry) {
    return entry.itemOrSpell == itemOrSpell && entry.conversionPoints >= 0;
  });
  if (it == end(entries))
    return std::nullopt;
  return it->conversionPoints;
}

std::optional<std::pair<int, bool>> Inventory::removeImpl(ItemOrSpell itemOrSpell, bool forConversion, bool forSale)
{
  if (itemOrSpell == FoodItemOrSpell && numFood > 1)
  {
    --numFood;
    return {{3, true}};
  }
  std::vector<Entry>::iterator toRemove = end(entries);
  for (auto it = begin(entries); it != end(entries); ++it)
  {
    if (it->itemOrSpell == itemOrSpell && (!forConversion || it->conversionPoints >= 0))
    {
      // When selling, prefer highest price (secondary: lowest CP);
      // when converting, prefer highest CP (secondary: lowest price),
      // otherwise prefer lowest CP (secondary: lowest price).
      const bool select = [&] {
        if (toRemove == end(entries))
          return true;
        const int deltaPrice = it->price - toRemove->price;
        const int deltaCP = it->conversionPoints - toRemove->conversionPoints;
        if (forSale)
          return deltaPrice > 0 || (deltaPrice == 0 && deltaCP < 0);
        if (forConversion)
          return deltaCP > 0 || (deltaCP == 0 && deltaPrice < 0);
        return deltaCP < 0 || (deltaCP == 0 && deltaPrice < 0);
      }();
      if (select)
        toRemove = it;
    }
  }
  if (toRemove == end(entries))
    return {};
  std::pair result = {toRemove->conversionPoints, toRemove->isSmall};
  entries.erase(toRemove);
  return {std::move(result)};
}

std::optional<std::pair<unsigned, bool>> Inventory::removeForConversion(ItemOrSpell itemOrSpell, bool magicAffinity)
{
  auto conversionResult = removeImpl(itemOrSpell, true, false);
  if (magicAffinity && conversionResult)
  {
    // All spells donate 10 of their conversion points
    for (auto& entry : entries)
    {
      if (std::holds_alternative<Spell>(entry.itemOrSpell) && entry.conversionPoints >= 10)
      {
        entry.conversionPoints -= 10;
        conversionResult->first += 10;
      }
    }
  }
  return conversionResult;
}

bool Inventory::remove(ItemOrSpell itemOrSpell)
{
  return removeImpl(itemOrSpell, false, false).has_value();
}

int Inventory::buyingPrice(Item item) const
{
  const auto thePrice = price(item);
  if (thePrice <= 0 || !negotiator)
    return thePrice;
  return std::max(1, thePrice - 5);
}

int Inventory::sellingPrice(ItemOrSpell itemOrSpell) const
{
  int highestPrice = -1;
  for (const auto& entry : entries)
  {
    if (entry.itemOrSpell == itemOrSpell && entry.price > highestPrice)
      highestPrice = entry.price;
  }
  return highestPrice;
}

unsigned Inventory::numFreeSmallSlots() const
{
  unsigned roomTaken = getSpells().size() * (spellsSmall ? 1 : LargeItemSize);
  for (const auto& [entry, _] : getItemsGrouped())
    roomTaken += entry.isSmall ? 1 : LargeItemSize;
  return numSlots * LargeItemSize - roomTaken;
}

bool Inventory::hasRoomFor(ItemOrSpell itemOrSpell) const
{
  bool smallItem;
  if (const auto item = std::get_if<Item>(&itemOrSpell))
  {
    // Potions and food can be grouped in inventory, return true if such an item is already present
    if (const auto potion = std::get_if<Potion>(&*item); potion && has(*potion))
      return true;
    if (numFood > 0 && *item == FoodItem)
    {
      assert(has(FoodItemOrSpell));
      return true;
    }
    smallItem = isSmall(*item) && !allItemsLarge;
  }
  else
    smallItem = spellsSmall && !allItemsLarge;
  const auto itemSize = smallItem ? 1 : LargeItemSize;
  return numFreeSmallSlots() >= itemSize;
}

bool Inventory::compress(ItemOrSpell itemOrSpell)
{
  auto entry = std::find_if(begin(entries), end(entries),
                            [&itemOrSpell](auto& entry) { return entry.itemOrSpell == itemOrSpell && !entry.isSmall; });
  if (entry == end(entries))
    return false;
  entry->isSmall = true;
  return true;
}

bool Inventory::transmute(ItemOrSpell itemOrSpell)
{
  const auto item = std::get_if<Item>(&itemOrSpell);
  // A few items cannot be transmuted, signified by a negative price.
  // Spells always have a price of 0.
  const auto price = item ? sellingPrice(*item) : 0;
  if (price < 0 || !removeImpl(itemOrSpell, false, true))
    return false;
  gold += static_cast<unsigned>(price);
  return true;
}

bool Inventory::translocate(Item shopItem)
{
  // Do not limit shopItem to actual ShopItems, as there are puzzle levels where other items are for sale
  entries.emplace_back(Entry{ItemOrSpell{shopItem}, isSmall(shopItem) && !allItemsLarge, price(shopItem),
                             initialConversionPoints(shopItem) / 2});
  return true;
}

void Inventory::clear()
{
  entries.clear();
  numFood = 0;
  fireHeartCharge = 0;
  crystalBallCharge = 10;
  crystalBallCosts = 4;
  triswordDamage = 2;
}

void Inventory::chargeFireHeart()
{
  fireHeartCharge += 5;
  if (fireHeartCharge > 100)
    fireHeartCharge = 100;
}

unsigned Inventory::fireHeartUsed()
{
  return std::exchange(fireHeartCharge, 0);
}

unsigned Inventory::getFireHeartCharge() const
{
  return fireHeartCharge;
}

void Inventory::chargeCrystalBall()
{
  ++crystalBallCharge;
}

unsigned Inventory::crystalBallUsed()
{
  crystalBallCosts += 2;
  return std::exchange(crystalBallCharge, 0);
}

unsigned Inventory::getCrystalBallCharge() const
{
  return crystalBallCharge;
}

unsigned Inventory::getCrystalBallUseCosts() const
{
  return crystalBallCosts;
}

int Inventory::chargeTrisword()
{
  return 7 - std::exchange(triswordDamage, 7);
}

bool Inventory::triswordUsed()
{
  if (triswordDamage > 2)
  {
    --triswordDamage;
    return true;
  }
  return false;
}

int Inventory::getTriswordDamage() const
{
  return triswordDamage;
}

unsigned Inventory::enchantPrayerBeads()
{
  unsigned count = 0;
  for (auto& entry : entries)
  {
    if (entry.itemOrSpell == ItemOrSpell{MiscItem::PrayerBead})
    {
      entry.itemOrSpell = MiscItem::EnchantedPrayerBead;
      ++count;
    }
  }
  return count;
}

bool Inventory::addFood(unsigned amount)
{
  if (numFood == 0 && amount > 0)
  {
    // Add only one item of Food to inventory, keep track of amount in numFood variable
    if (!add(FoodItemOrSpell))
      return false;
    --amount;
  }
  numFood += amount;
  return true;
}

unsigned Inventory::getFoodCount() const
{
  return numFood;
}

unsigned Inventory::tryConsumeFood(unsigned amount)
{
  if (numFood < amount)
  {
    amount -= numFood;
    numFood = 0;
    remove(FoodItemOrSpell);
    return amount;
  }
  if (amount > 0)
  {
    numFood -= amount;
    if (numFood == 0)
      remove(FoodItemOrSpell);
  }
  return 0;
}

auto Inventory::getItemsAndSpells() const -> const std::vector<Entry>&
{
  return entries;
}

auto Inventory::getItemsGrouped() const -> std::vector<std::pair<Entry, int>>
{
  std::vector<std::pair<Entry, int>> itemsGrouped;
  for (const auto& entry : entries)
  {
    const auto item = std::get_if<Item>(&entry.itemOrSpell);
    if (!item)
      continue;
    // Food for Goatperson is handled separately
    if (*item == FoodItem)
    {
      itemsGrouped.emplace_back(entry, numFood);
      continue;
    }
    // Only potions can be grouped
    if (!std::holds_alternative<Potion>(*item))
    {
      itemsGrouped.emplace_back(entry, 1);
      continue;
    }
    auto pairToUpdate =
        std::find_if(begin(itemsGrouped), end(itemsGrouped), [item = ItemOrSpell{*item}](const auto& groupedEntry) {
          return groupedEntry.first.itemOrSpell == item;
        });
    if (pairToUpdate == end(itemsGrouped))
      itemsGrouped.emplace_back(entry, 1);
    else
    {
      auto& itemToUpdate = pairToUpdate->first;
      itemToUpdate.conversionPoints = std::max(itemToUpdate.conversionPoints, entry.conversionPoints);
      ++pairToUpdate->second;
    }
  }
  return itemsGrouped;
}

auto Inventory::getSpells() const -> std::vector<Entry>
{
  std::vector<Entry> spells(entries.size());
  auto it = std::copy_if(begin(entries), end(entries), begin(spells),
                         [](const auto& entry) { return entry.itemOrSpell.index() == 1; });
  spells.erase(it, end(spells));
  return spells;
}

namespace
{
  template <class T>
  std::vector<std::pair<T, int>> getEntryCountsOrdered(const std::vector<Inventory::Entry>& entries)
  {
    std::map<T, int> counts;
    std::vector<T> filteredEntries;
    for (const auto& entry : entries)
    {
      if (const T* filtered = std::get_if<T>(&entry.itemOrSpell))
      {
        filteredEntries.push_back(*filtered);
        ++counts[*filtered];
      }
    }
    std::vector<std::pair<T, int>> result;
    result.reserve(counts.size());
    for (const auto& entry : filteredEntries)
    {
      auto& count = counts[entry];
      if (count > 0)
      {
        result.emplace_back(std::pair{entry, count});
        count = 0;
      }
    }
    assert(result.size() == counts.size());
    return result;
  }
} // namespace

std::vector<std::pair<Item, int>> Inventory::getItemCounts() const
{
  auto counts = getEntryCountsOrdered<Item>(entries);
  if (numFood > 1)
  {
    std::for_each(begin(counts), end(counts), [numFood = numFood](auto& itemCount) {
      if (itemCount.first == FoodItem)
        itemCount.second = numFood;
    });
  }
  return counts;
}

std::vector<std::pair<Spell, int>> Inventory::getSpellCounts() const
{
  return getEntryCountsOrdered<Spell>(entries);
}
