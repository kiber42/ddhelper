#pragma once

#include <optional>
#include <variant>
#include <vector>

enum class Item;
enum class Spell;

class Inventory
{
public:
  explicit Inventory(int numSlots = 6, int spellConversionPoints = 100, bool spellsSmall = false, bool allItemsLarge = false);

  void add(Item item);
  // Returns conversion value of item; or nullopt if item was not in inventory
  std::optional<int> remove(Item item);
  bool has(Item item) const;
  bool hasRoomFor(Item item) const;

  void add(Spell spell);
  void addFree(Spell spell);
  // Returns conversion value of spell; or nullopt if spell was not in inventory
  std::optional<int> remove(Spell spell, bool magicAffinity = false);
  bool has(Spell spell) const;
  bool hasRoomFor(Spell spell) const;

  int smallSlotsLeft() const;
  void clear();

  // Replace prayer beads by enchanted prayer beads, return number of beads enchanted
  int enchantPrayerBeads();

  using ItemOrSpell = std::variant<Item, Spell>;
  struct Entry
  {
    ItemOrSpell itemOrSpell;
    bool isSmall;
    int count;
    int conversionPoints;
  };

  std::vector<Entry> getEntries() const;
  std::vector<Entry> getItems() const;
  std::vector<Entry> getSpells() const;

  int gold;

private:
  std::vector<Entry> entries;
  std::vector<Entry>::iterator find(ItemOrSpell itemOrSpell);
  std::vector<Entry>::const_iterator find(ItemOrSpell itemOrSpell) const;

  int numSlots;
  int spellConversionPoints;
  bool spellsSmall;
  bool allItemsLarge;
};
