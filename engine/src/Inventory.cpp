#include "engine/Inventory.hpp"

#include "engine/Items.hpp"
#include "engine/Magic.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

Inventory::Inventory(
    int numSlots, int spellConversionPoints, bool spellsSmall, bool allItemsLarge, bool hasNegotiatorTrait)
  : gold(20)
  , numSlots(numSlots)
  , spellConversionPoints(spellConversionPoints)
  , spellsSmall(spellsSmall)
  , allItemsLarge(allItemsLarge)
  , negotiator(hasNegotiatorTrait)
  , fireHeartCharge(0)
  , crystalBallCharge(10)
  , crystalBallCosts(4)
  , triswordDamage(2)
{
  addFree(Potion::HealthPotion);
  addFree(Potion::ManaPotion);
}

void Inventory::add(ItemOrSpell itemOrSpell)
{
  const auto [smallItem, price, conversionPoints] = [&, item = std::get_if<Item>(&itemOrSpell)] {
    if (item)
      return std::tuple{isSmall(*item) && !allItemsLarge, buyingPrice(*item), initialConversionPoints(*item)};
    else
      return std::tuple{spellsSmall && !allItemsLarge, 0, spellConversionPoints};
  }();
  entries.emplace_back(Entry{itemOrSpell, smallItem, price, conversionPoints});
}

void Inventory::addFree(Item item)
{
  entries.emplace_back(Entry{ItemOrSpell{item}, isSmall(item) && !allItemsLarge, 0, initialConversionPoints(item)});
}

void Inventory::addFree(Spell spell)
{
  entries.emplace_back(Entry{spell, spellsSmall && !allItemsLarge, 0, 0});
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

std::optional<int> Inventory::getConversionPoints(ItemOrSpell itemOrSpell) const
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

std::optional<std::pair<int, bool>> Inventory::removeForConversion(ItemOrSpell itemOrSpell, bool magicAffinity)
{
  auto conversionResult = removeImpl(itemOrSpell, true, false);
  if (magicAffinity && conversionResult)
  {
    // All spells donate 10 of their conversion points
    for (auto& entry : entries)
    {
      if (std::get_if<Spell>(&entry.itemOrSpell) && entry.conversionPoints >= 10)
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

namespace
{
  constexpr int LargeItemSize = 5;
}

int Inventory::numFreeSmallSlots() const
{
  int roomTaken = getSpells().size() * (spellsSmall ? 1 : LargeItemSize);
  for (const auto& [entry, _] : getItemsGrouped())
    roomTaken += entry.isSmall ? 1 : LargeItemSize;
  return numSlots * LargeItemSize - roomTaken;
}

bool Inventory::hasRoomFor(ItemOrSpell itemOrSpell) const
{
  bool smallItem;
  if (const auto item = std::get_if<Item>(&itemOrSpell))
  {
    if (auto potion = std::get_if<Potion>(&*item); potion && has(*potion))
      return true;
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
  gold += price;
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
}

void Inventory::chargeFireHeart()
{
  fireHeartCharge += 5;
  if (fireHeartCharge > 100)
    fireHeartCharge = 100;
}

int Inventory::fireHeartUsed()
{
  return std::exchange(fireHeartCharge, 0);
}

int Inventory::getFireHeartCharge() const
{
  return fireHeartCharge;
}

void Inventory::chargeCrystalBall()
{
  ++crystalBallCharge;
}

int Inventory::crystalBallUsed()
{
  crystalBallCosts += 2;
  return std::exchange(crystalBallCharge, 0);
}

int Inventory::getCrystalBallCharge() const
{
  return crystalBallCharge;
}

int Inventory::getCrystalBallUseCosts() const
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

int Inventory::enchantPrayerBeads()
{
  int count = 0;
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
    // Only potions can be grouped (TODO: Food for Goatsperson not implemented yet)
    if (std::get_if<Potion>(&*item) == nullptr)
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
  return getEntryCountsOrdered<Item>(entries);
}

std::vector<std::pair<Spell, int>> Inventory::getSpellCounts() const
{
  return getEntryCountsOrdered<Spell>(entries);
}
