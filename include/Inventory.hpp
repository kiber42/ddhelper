#pragma once

#include <variant>
#include <vector>

enum class Item;
enum class Spell;

class Inventory
{
public:
  explicit Inventory(int numSlots = 6, int spellConversionPoints = 100, bool spellsSmall = false, bool allItemsLarge = false);

  void add(Item item);
  bool remove(Item item);
  bool has(Item item) const;
  bool hasRoomFor(Item item) const;

  void add(Spell spell);
  bool remove(Spell spell);
  bool has(Spell spell) const;
  bool hasRoomFor(Spell spell) const;

  int smallSlotsLeft() const;
  void clear();

private:
  using ItemOrSpell = std::variant<Item, Spell>;
  struct Entry
  {
    ItemOrSpell itemOrSpell;
    bool isSmall;
    int count;
    int conversionPoints;
  };

  std::vector<Entry> entries;
  std::vector<Entry>::iterator find(ItemOrSpell itemOrSpell);
  std::vector<Entry>::const_iterator find(ItemOrSpell itemOrSpell) const;

  int numSlots;
  int spellConversionPoints;
  bool spellsSmall;
  bool allItemsLarge;
};
