#include "Inventory.hpp"

#include "Items.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <numeric>

Inventory::Inventory(int numSlots, int spellConversionPoints, bool spellsSmall, bool allItemsLarge)
  : numSlots(numSlots)
  , spellConversionPoints(spellConversionPoints)
  , spellsSmall(spellsSmall)
  , allItemsLarge(allItemsLarge)
{
  add(Item::HealthPotion);
  add(Item::ManaPotion);
}

void Inventory::add(Item item)
{
  if (canGroup(item))
  {
    auto it = find(item);
    if (it != end(entries))
    {
      it->count++;
      return;
    }
  }
  entries.emplace_back(Entry{{item}, !allItemsLarge && isSmall(item), 1, conversionPointsInitial(item)});
}

std::optional<int> Inventory::remove(Item item)
{
  auto it = find(item);
  if (it == end(entries))
    return std::nullopt;
  const int conversionPoints = it->conversionPoints;
  if (conversionPoints < 0)
    return std::nullopt;
  it->count--;
  if (it->count <= 0)
    entries.erase(it);
  return conversionPoints;
}

bool Inventory::has(Item item) const
{
  return find(item) != end(entries);
}

bool Inventory::hasRoomFor(Item item) const
{
  const int smallSlotsNeeded = !allItemsLarge && isSmall(item) ? 1 : 5;
  return smallSlotsNeeded <= smallSlotsLeft();
}

void Inventory::add(Spell spell)
{
  entries.emplace_back(Entry{spell, !allItemsLarge && spellsSmall, 1, spellConversionPoints});
}

void Inventory::addFree(Spell spell)
{
  entries.emplace_back(Entry{spell, !allItemsLarge && spellsSmall, 1, 0});
}

std::optional<int> Inventory::remove(Spell spell, bool magicAffinity)
{
  auto it = find(spell);
  if (it == end(entries))
    return std::nullopt;
  int conversionPoints = it->conversionPoints;
  if (conversionPoints < 0)
    return std::nullopt;
  entries.erase(it);
  if (magicAffinity)
  {
    // All remaining spells donate 10 of their conversion points
    for (auto& entry : entries)
    {
      if (entry.itemOrSpell.index() == 0 /* Item */ || entry.conversionPoints < 10)
        continue;
      entry.conversionPoints -= 10;
      conversionPoints += 10;
    }
  }
  return conversionPoints;
}

bool Inventory::has(Spell spell) const
{
  return find(spell) != end(entries);
}

bool Inventory::hasRoomFor(Spell spell) const
{
  const int smallSlotsNeeded = !allItemsLarge && spellsSmall ? 1 : 5;
  return smallSlotsNeeded <= smallSlotsLeft();
}

int Inventory::smallSlotsLeft() const
{
  const int roomTaken = std::transform_reduce(begin(entries), end(entries), 0, std::plus<>(),
                                              [](auto& entry) { return entry.isSmall ? 1 : 5; });
  return numSlots * 5 - roomTaken;
}

void Inventory::clear()
{
  entries.clear();
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
