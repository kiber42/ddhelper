#include "Inventory.hpp"

#include "Items.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

Inventory::Inventory(int numSlots, int spellConversionPoints, bool spellsSmall, bool allItemsLarge)
  : gold(20)
  , numSlots(numSlots)
  , spellConversionPoints(spellConversionPoints)
  , spellsSmall(spellsSmall)
  , allItemsLarge(allItemsLarge)
  , fireHeartCharge(0)
  , crystalBallCharge(10)
  , crystalBallCosts(4)
  , triswordDamage(2)
{
  add(Item::FreeHealthPotion);
  add(Item::FreeManaPotion);
}

void Inventory::add(ItemOrSpell itemOrSpell)
{
  const auto item = std::get_if<Item>(&itemOrSpell);
  int conversionPoints = item ? conversionPointsInitial(*item) : spellConversionPoints;
  entries.emplace_back(Entry{itemOrSpell, isInitiallySmall(itemOrSpell), conversionPoints});
}

void Inventory::addFree(Spell spell)
{
  entries.emplace_back(Entry{spell, isInitiallySmall(spell), 0});
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

std::optional<std::pair<int, bool>> Inventory::removeForConversion(ItemOrSpell itemOrSpell, bool magicAffinity)
{
  auto it = find(itemOrSpell);
  if (it == end(entries))
    return std::nullopt;
  int conversionPoints = it->conversionPoints;
  if (conversionPoints < 0)
    return std::nullopt;
  entries.erase(it);
  if (magicAffinity && itemOrSpell.index() == 1)
  {
    // All remaining spells donate 10 of their conversion points
    for (auto& entry : entries)
    {
      if (entry.itemOrSpell.index() == 1 && entry.conversionPoints >= 10)
      {
        entry.conversionPoints -= 10;
        conversionPoints += 10;
      }
    }
  }
  return {{conversionPoints, isSmall}};
}

bool Inventory::remove(ItemOrSpell itemOrSpell)
{
  auto it = find(itemOrSpell);
  if (it == end(entries))
    return false;
  entries.erase(it);
  return true;
}

bool Inventory::isInitiallySmall(ItemOrSpell itemOrSpell) const
{
  if (allItemsLarge)
    return false;
  if (const auto item = std::get_if<Item>(&itemOrSpell))
    return isSmall(*item);
  return spellsSmall;
}

namespace
{
  constexpr int LargeItemSize = 5;
}

int Inventory::numFreeSmallSlots() const
{
  int roomTaken = getSpells().size();
  for (const auto& [entry, _] : getItemsGrouped())
    roomTaken += entry.isSmall ? 1 : LargeItemSize;
  return numSlots * LargeItemSize - roomTaken;
}

bool Inventory::hasRoomFor(ItemOrSpell itemOrSpell) const
{
  if (has(itemOrSpell))
  {
    auto item = std::get_if<Item>(&itemOrSpell);
    if (item && canGroup(*item))
      return true;
  }
  return numFreeSmallSlots() >= (isInitiallySmall(itemOrSpell) ? 1 : LargeItemSize);
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
    auto pairToUpdate = [&itemsGrouped, item = *item] {
      if (!canGroup(item))
        return end(itemsGrouped);
      return std::find_if(begin(itemsGrouped), end(itemsGrouped), [item = ItemOrSpell{item}](const auto& groupedEntry) {
        return groupedEntry.first.itemOrSpell == item;
      });
    }();
    if (pairToUpdate == end(itemsGrouped))
      itemsGrouped.emplace_back(entry, 1);
    else
    {
      pairToUpdate->first.conversionPoints = std::max(pairToUpdate->first.conversionPoints, entry.conversionPoints);
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
      const auto filtered = std::get_if<T>(&entry.itemOrSpell);
      if (filtered)
      {
        ++counts[*filtered];
        filteredEntries.emplace_back(*filtered);
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
                      [&itemOrSpell](auto& entry) { return entry.itemOrSpell == itemOrSpell; });
}

auto Inventory::find(ItemOrSpell itemOrSpell) const -> std::vector<Entry>::const_iterator
{
  return std::find_if(begin(entries), end(entries),
                      [&itemOrSpell](auto& entry) { return entry.itemOrSpell == itemOrSpell; });
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
