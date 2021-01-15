#pragma once

#include <optional>
#include <variant>
#include <vector>

enum class Item;
enum class Spell;

using ItemOrSpell = std::variant<Item, Spell>;

class Inventory
{
public:
  explicit Inventory(int numSlots = 6,
                     int spellConversionPoints = 100,
                     bool spellsSmall = false,
                     bool allItemsLarge = false);

  // Add item or spell to inventory (currently this does not check space requirements)
  void add(ItemOrSpell itemOrSpell);
  // Add spell to inventory, set its conversion value to 0
  void addFree(Spell spell);

  bool has(ItemOrSpell itemOrSpell) const;

  // Returns true if item or spell is in inventory and can be converted
  bool canConvert(ItemOrSpell itemOrSpell) const;

  // Returns conversion value of item or spell, or nullopt if item is not in inventory or cannot be converted
  std::optional<int> getConversionPoints(ItemOrSpell itemOrSpell) const;

  // Tries to remove an item, returns nullopt if item is not in inventory or cannot be converted.
  // On success, returns conversion value and whether the item was small.
  std::optional<std::pair<int, bool>> removeForConversion(ItemOrSpell itemOrSpell, bool magicAffinity = false);

  // If item or spell is present in inventory, remove it and return true; false otherwise.
  // Also removes non-convertable items.
  bool remove(ItemOrSpell itemOrSpell);

  // Returns whether an item or spell would be small upon entering the inventory, accounting for class effects
  bool isInitiallySmall(ItemOrSpell itemOrSpell) const;

  // Return number of free small inventory slots
  int numFreeSmallSlots() const;

  // Check if there is enough space in the inventory for an item or a spell
  bool hasRoomFor(ItemOrSpell itemOrSpell) const;

  // Remove all items and spells from inventory
  void clear();

  // Helper functions for inventory items with special behaviours
  void chargeFireHeart();
  [[nodiscard]] int fireHeartUsed();
  int getFireHeartCharge() const;

  void chargeCrystalBall();
  [[nodiscard]] int crystalBallUsed();
  int getCrystalBallCharge() const;
  int getCrystalBallUseCosts() const;

  [[nodiscard]] int chargeTrisword();
  [[nodiscard]] bool triswordUsed();
  int getTriswordDamage() const;

  // Replace prayer beads by enchanted prayer beads, return number of beads enchanted
  int enchantPrayerBeads();

  struct Entry
  {
    ItemOrSpell itemOrSpell;
    bool isSmall;
    int count;
    int conversionPoints;
  };

  std::vector<Entry> getItems() const;
  std::vector<Entry> getSpells() const;
  const std::vector<Entry>& getItemsAndSpells() const;

  int gold;

private:
  std::vector<Entry> entries;
  std::vector<Entry>::iterator find(ItemOrSpell itemOrSpell);
  std::vector<Entry>::const_iterator find(ItemOrSpell itemOrSpell) const;

  int numSlots;
  int spellConversionPoints;
  bool spellsSmall;
  bool allItemsLarge;

  int fireHeartCharge;
  int crystalBallCharge;
  int crystalBallCosts;
  int triswordDamage;
};
