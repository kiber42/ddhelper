#include "Inventory.hpp"

#include "Items.hpp"
#include "Spells.hpp"

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
  , numFreeHealthPotions(1)
  , numFreeManaPotions(1)
  , fireHeartCharge(0)
  , crystalBallCharge(10)
  , crystalBallCosts(4)
  , triswordDamage(2)
{
  add(Item::HealthPotion);
  add(Item::ManaPotion);
}

void Inventory::add(ItemOrSpell itemOrSpell)
{
  bool smallItem;
  int conversionPoints;
  if (const auto item = std::get_if<Item>(&itemOrSpell))
  {
    smallItem = isSmall(*item) && !allItemsLarge;
    conversionPoints = conversionPointsInitial(*item);
  }
  else
  {
    smallItem = spellsSmall && !allItemsLarge;
    conversionPoints = spellConversionPoints;
  }
  entries.emplace_back(Entry{itemOrSpell, smallItem, conversionPoints});
}

void Inventory::addFree(Spell spell)
{
  entries.emplace_back(Entry{spell, spellsSmall && !allItemsLarge, 0});
}

bool Inventory::has(ItemOrSpell itemOrSpell) const
{
  return find(itemOrSpell) != end(entries);
}

bool Inventory::canConvert(ItemOrSpell itemOrSpell) const
{
  const auto it = find(itemOrSpell);
  return it != end(entries) && it->conversionPoints >= 0;
}

std::optional<int> Inventory::getConversionPoints(ItemOrSpell itemOrSpell) const
{
  const auto it = find(itemOrSpell);
  if (it == end(entries) || it->conversionPoints < 0)
    return std::nullopt;
  return it->conversionPoints;
}

std::optional<std::pair<int, bool>> Inventory::removeImpl(ItemOrSpell itemOrSpell, bool forConversion, bool forSale)
{
  auto it = find(itemOrSpell);
  if (it == end(entries))
    return {};
  const int conversionPoints = it->conversionPoints;
  if (forConversion && conversionPoints < 0)
    return {};
  const bool wasSmall = it->isSmall;
  // Prices of initial potions are special. Prefer removing the cheaper versions from inventory, except when selling.
  if (const auto item = std::get_if<Item>(&itemOrSpell); item)
  {
    if (*item == Item::HealthPotion)
    {
      if (numFreeHealthPotions > 0 && (!forSale || onlyHaveFreeHealthPotions()))
        --numFreeHealthPotions;
    }
    else if (*item == Item::ManaPotion)
    {
      if (numFreeManaPotions > 0 && (!forSale || onlyHaveFreeManaPotions()))
        --numFreeManaPotions;
    }
  }
  entries.erase(it);
  return {{conversionPoints, wasSmall}};
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
  return bool(removeImpl(itemOrSpell, false, false));
}

int Inventory::buyingPrice(Item item) const
{
  const auto thePrice = price(item);
  if (thePrice <= 0 || !negotiator)
    return thePrice;
  return std::max(1, thePrice - 5);
}

int Inventory::sellingPrice(Item item) const
{
  const auto thePrice = buyingPrice(item);
  if (item == Item::HealthPotion)
  {
    if (numFreeHealthPotions > 0 && onlyHaveFreeHealthPotions())
      return 0;
  }
  else if (item == Item::ManaPotion)
  {
    if (numFreeManaPotions > 0 && onlyHaveFreeManaPotions())
      return 0;
  }
  return thePrice;
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
    if (canGroup(*item) && has(itemOrSpell))
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
  assert(!isPotion(shopItem));
  entries.emplace_back(Entry{shopItem, isSmall(shopItem) && !allItemsLarge, conversionPointsInitial(shopItem) / 2});
  return true;
}

void Inventory::clear()
{
  entries.clear();
  numFreeHealthPotions = 0;
  numFreeManaPotions = 0;
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
    if (entry.itemOrSpell == ItemOrSpell{Item::PrayerBead})
    {
      entry.itemOrSpell = Item::EnchantedPrayerBead;
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
    if (!canGroup(*item))
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

auto Inventory::find(ItemOrSpell itemOrSpell) -> std::vector<Entry>::iterator
{
  return std::find_if(begin(entries), end(entries),
                      [itemOrSpell](auto& entry) { return entry.itemOrSpell == itemOrSpell; });
}

auto Inventory::find(ItemOrSpell itemOrSpell) const -> std::vector<Entry>::const_iterator
{
  return std::find_if(begin(entries), end(entries),
                      [itemOrSpell](auto& entry) { return entry.itemOrSpell == itemOrSpell; });
}

bool Inventory::onlyHaveFreeHealthPotions() const
{
  const int numHealthPotions =
      std::count_if(begin(entries), end(entries), [search = ItemOrSpell{Item::HealthPotion}](const Entry& entry) {
        return entry.itemOrSpell == search;
      });
  assert(numHealthPotions >= numFreeHealthPotions);
  return numHealthPotions == numFreeHealthPotions;
}

bool Inventory::onlyHaveFreeManaPotions() const
{
  const int numManaPotions =
      std::count_if(begin(entries), end(entries), [search = ItemOrSpell{Item::ManaPotion}](const Entry& entry) {
        return entry.itemOrSpell == search;
      });
  assert(numManaPotions >= numFreeManaPotions);
  return numManaPotions == numFreeManaPotions;
}

template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

std::string toString(ItemOrSpell itemOrSpell)
{
  return std::visit(overloaded{[](Item item) { return toString(item); }, [](Spell spell) { return toString(spell); }},
                    itemOrSpell);
}
