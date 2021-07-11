#pragma once

#include "engine/DungeonSetup.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

using ItemOrSpell = std::variant<Item, Spell>;

class Inventory
{
public:
  explicit Inventory(const DungeonSetup&);

  // Add item or spell to inventory (currently this does not check space requirements)
  void add(ItemOrSpell);
  // Add item to inventory, set its selling price to 0 (conversion points are unchanged)
  void addFree(Item);
  // Add spell to inventory, set its conversion value to 0 (price is always 0)
  void addFree(Spell);

  bool has(ItemOrSpell) const;

  // Returns true if item or spell is in inventory and can be converted
  bool canConvert(ItemOrSpell) const;

  // Returns conversion value of item or spell, or nullopt if item is not in inventory or cannot be converted
  std::optional<unsigned> getConversionPoints(ItemOrSpell) const;

  // Tries to remove an item, returns nullopt if item is not in inventory or cannot be converted.
  // On success, returns conversion value and whether the item was small.
  std::optional<std::pair<unsigned, bool>> removeForConversion(ItemOrSpell, bool magicAffinity = false);

  // If item or spell is present in inventory, remove it and return true; false otherwise.
  // No additional restrictions are applied: also removes non-convertable items and items that cannot be transmuted.
  bool remove(ItemOrSpell);

  // Return the shop price of an item; can differ from the regular price for negotiator.
  int buyingPrice(Item) const;

  // Return the price of an item when "selling" it (by using a transmutation scroll). Spell prices are always 0.
  int sellingPrice(ItemOrSpell) const;

  // Return number of free small inventory slots
  unsigned numFreeSmallSlots() const;

  // Check if there is enough space in the inventory for an item or a spell
  bool hasRoomFor(ItemOrSpell) const;

  // Mark an item or spell as small.  Returns false if no matching normal-sized entry was found in inventory.
  bool compress(ItemOrSpell);

  // Remove an item and receive its cost in gold.  Returns false if item not in inventory or if it cannot be transmuted.
  bool transmute(ItemOrSpell);

  // Add item obtained using a Translocation Seal, i.e. with half its usual conversion points (rounded down)
  bool translocate(Item);

  // Remove all items and spells from inventory
  void clear();

  // Helper functions for inventory items with special behaviours
  void chargeFireHeart();
  [[nodiscard]] unsigned fireHeartUsed();
  unsigned getFireHeartCharge() const;

  void chargeCrystalBall();
  [[nodiscard]] unsigned crystalBallUsed();
  unsigned getCrystalBallCharge() const;
  unsigned getCrystalBallUseCosts() const;

  [[nodiscard]] int chargeTrisword();
  [[nodiscard]] bool triswordUsed();
  int getTriswordDamage() const;

  // Replace prayer beads by enchanted prayer beads, return number of beads enchanted
  unsigned enchantPrayerBeads();

  void addFood(unsigned amount);
  [[nodiscard]] unsigned getFoodCount() const;
  // Try to consume requested amount of food.  Returns 0 on success.  Otherwise, all food is consumed and the missing amount is returned.
  [[nodiscard]] unsigned tryConsumeFood(unsigned amount);

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

  unsigned gold{20};

private:
  std::vector<Entry> entries;

  std::optional<std::pair<int, bool>> removeImpl(ItemOrSpell itemOrSpell, bool forConversion, bool forSale);

  unsigned numSlots{6};
  int spellConversionPoints{100};
  bool spellsSmall{false};
  bool allItemsLarge{false};
  bool negotiator{false};

  unsigned numFood{0};
  unsigned fireHeartCharge{0};
  unsigned crystalBallCharge{10};
  unsigned crystalBallCosts{4};
  int triswordDamage{2};
};

template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

constexpr const char* toString(ItemOrSpell itemOrSpell)
{
  return std::visit(
      overloaded{[](const Item& item) { return toString(item); }, [](const Spell& spell) { return toString(spell); }},
      itemOrSpell);
}
