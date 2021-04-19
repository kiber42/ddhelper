#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class Item;
enum class Spell;

using ItemOrSpell = std::variant<Item, Spell>;

std::string toString(ItemOrSpell itemOrSpell);

class Inventory
{
public:
  explicit Inventory(int numSlots = 6,
                     int spellConversionPoints = 100,
                     bool spellsSmall = false,
                     bool allItemsLarge = false,
                     bool hasNegotiatorTrait = false);

  // Add item or spell to inventory (currently this does not check space requirements)
  void add(ItemOrSpell itemOrSpell);
  // Add item to inventory, set its selling price to 0 (conversion points are unchanged)
  void addFree(Item item);
  // Add spell to inventory, set its conversion value to 0 (price is always 0)
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
  // No additional restrictions are applied: also removes non-convertable items and items that cannot be transmuted.
  bool remove(ItemOrSpell itemOrSpell);

  // Return the shop price of an item; can differ from the regular price for negotiator.
  int buyingPrice(Item item) const;

  // Return the price of an item when "selling" it (by using a transmutation scroll). Spell prices are always 0.
  int sellingPrice(ItemOrSpell itemOrSpell) const;

  // Return number of free small inventory slots
  int numFreeSmallSlots() const;

  // Check if there is enough space in the inventory for an item or a spell
  bool hasRoomFor(ItemOrSpell itemOrSpell) const;

  // Mark an item or spell as small.  Returns false if no matching normal-sized entry was found in inventory.
  bool compress(ItemOrSpell itemOrSpell);

  // Remove an item and receive its cost in gold.  Returns false if item not in inventory or if it cannot be transmuted.
  bool transmute(ItemOrSpell itemOrSpell);

  // Add item obtained using a Translocation Seal, i.e. with half its usual conversion points (rounded down)
  bool translocate(Item itemOrSpell);

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
    int price;
    int conversionPoints;
  };

  // List all items and spell entries in inventory without any modification
  const std::vector<Entry>& getItemsAndSpells() const;
  // Return list of all items in inventory, group items that can be grouped
  std::vector<std::pair<Entry, int>> getItemsGrouped() const;
  // Return list of all spells in inventory
  std::vector<Entry> getSpells() const;
  // For each type of item in inventory, return how many there are
  std::vector<std::pair<Item, int>> getItemCounts() const;
  // For each type of spell in inventory, return how many there are
  std::vector<std::pair<Spell, int>> getSpellCounts() const;

  int gold;

private:
  std::vector<Entry> entries;

  std::optional<std::pair<int, bool>> removeImpl(ItemOrSpell itemOrSpell, bool forConversion, bool forSale);

  int numSlots;
  int spellConversionPoints;
  bool spellsSmall;
  bool allItemsLarge;
  bool negotiator;

  int fireHeartCharge;
  int crystalBallCharge;
  int crystalBallCosts;
  int triswordDamage;
};
