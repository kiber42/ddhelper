#include "bandit/bandit.h"

#include "engine/Hero.hpp"
#include "engine/Inventory.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
}

void testInventory()
{
  describe("Inventory", [] {
    Inventory inv{DungeonSetup{}};
    it("shall have free health and mana potion by default", [&] {
      AssertThat(inv.has(Potion::HealthPotion), IsTrue());
      AssertThat(inv.has(Potion::ManaPotion), IsTrue());
    });
    it("shall report correct selling prices for potions", [&] {
      AssertThat(inv.sellingPrice(Potion::HealthPotion), Equals(0));
      AssertThat(inv.sellingPrice(Potion::ManaPotion), Equals(0));
      AssertThat(inv.add(Potion::HealthPotion), IsTrue());
      AssertThat(inv.sellingPrice(Potion::HealthPotion), Equals(10));
      AssertThat(inv.add(Potion::ManaPotion), IsTrue());
      AssertThat(inv.sellingPrice(Potion::ManaPotion), Equals(10));
    });
    it("shall have room for 30 small / 6 large items", [&] {
      AssertThat(inv.numFreeSmallSlots(), Equals(28u));
      inv.clear();
      AssertThat(inv.numFreeSmallSlots(), Equals(30u));
      AssertThat(inv.add(ShopItem::BadgeOfHonour), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(25u));
      AssertThat(inv.add(ShopItem::BloodySigil), IsTrue());
      AssertThat(inv.add(ShopItem::FineSword), IsTrue());
      AssertThat(inv.add(ShopItem::PendantOfHealth), IsTrue());
      AssertThat(inv.add(ShopItem::PendantOfMana), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(5u));
      AssertThat(inv.hasRoomFor(ShopItem::TowerShield), IsTrue());
      AssertThat(inv.add(ShopItem::Spoon), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(4u));
      AssertThat(inv.hasRoomFor(ShopItem::MagePlate), IsFalse());
      AssertThat(inv.hasRoomFor(ShopItem::TowerShield), IsFalse());
      AssertThat(inv.hasRoomFor(ShopItem::DragonSoul), IsTrue());
      AssertThat(inv.add(ShopItem::Spoon), IsTrue());
      AssertThat(inv.add(Potion::HealthPotion), IsTrue());
      AssertThat(inv.add(ShopItem::DragonSoul), IsTrue());
      AssertThat(inv.add(AlchemistSeal::CompressionSeal), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(0u));
    });
    it("shall account for grouping", [&] {
      inv.clear();
      AssertThat(inv.add(Potion::HealthPotion), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
      AssertThat(inv.add(Potion::HealthPotion), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
      AssertThat(inv.add(Potion::HealthPotion), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
      AssertThat(inv.addFree(Potion::HealthPotion), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
    });
    it("shall treat spells as large for most classes", [] {
      Hero hero(HeroClass::Sorcerer);
      AssertThat(hero.receive(Spell::Apheelsik), IsTrue());
      AssertThat(hero.receive(Spell::Bludtupowa), IsTrue());
      AssertThat(hero.receiveFreeSpell(Spell::Burndayraz), IsTrue());
      AssertThat(hero.receive(Spell::Bysseps), IsTrue());
      AssertThat(hero.hasRoomFor(Spell::Cydstepp), IsTrue());
      AssertThat(hero.receive(Spell::Cydstepp), IsTrue());
      AssertThat(hero.hasRoomFor(Spell::Endiswal), IsFalse());
      AssertThat(hero.hasRoomFor(Spell::Cydstepp), IsFalse());
    });
    it("shall treat spells as small for Wizards", [] {
      Hero hero(HeroClass::Wizard);
      AssertThat(hero.has(Spell::Burndayraz), IsTrue());
      AssertThat(hero.receiveFreeSpell(Spell::Apheelsik), IsTrue());
      AssertThat(hero.receive(Spell::Apheelsik), IsTrue());
      AssertThat(hero.receive(Spell::Bludtupowa), IsTrue());
      AssertThat(hero.receiveFreeSpell(Spell::Burndayraz), IsTrue());
      AssertThat(hero.receive(Spell::Bysseps), IsTrue());
      AssertThat(hero.receive(Spell::Cydstepp), IsTrue());
      AssertThat(hero.receive(Spell::Endiswal), IsTrue());
      AssertThat(hero.receive(Spell::Getindare), IsTrue());
      AssertThat(hero.receive(Spell::Halpmeh), IsTrue());
      AssertThat(hero.receive(Spell::Imawal), IsTrue());
      AssertThat(hero.receive(Spell::Lemmisi), IsTrue());
      AssertThat(hero.receive(Spell::Pisorf), IsTrue());
      AssertThat(hero.receive(Spell::Weytwut), IsTrue());
      AssertThat(hero.receive(Spell::Wonafyt), IsTrue());
      AssertThat(hero.receiveFreeSpell(Spell::Wonafyt), IsTrue());
      AssertThat(hero.receive(Spell::Wonafyt), IsTrue());
      AssertThat(hero.getItemsAndSpells().size(), Equals(19u));
      AssertThat(hero.getSpells().size(), Equals(17u));
      AssertThat(hero.getItemCounts().size(), Equals(2u));
      const auto spellCounts = hero.getSpellCounts();
      AssertThat(spellCounts.size(), Equals((unsigned)Spell::Last + 1));
      AssertThat(spellCounts.back().first, Equals(Spell::Wonafyt));
      AssertThat(spellCounts.back().second, Equals(3u));
      AssertThat(hero.hasRoomFor(ShopItem::KegOfMana), IsTrue());
      AssertThat(hero.receive(ShopItem::KegOfMana), IsTrue());
      AssertThat(hero.hasRoomFor(ShopItem::WickedGuitar), IsTrue());
      AssertThat(hero.receive(ShopItem::WickedGuitar), IsTrue());
      AssertThat(hero.hasRoomFor(ShopItem::OrbOfZot), IsFalse());
      AssertThat(hero.hasRoomFor(Potion::QuicksilverPotion), IsTrue());
      AssertThat(hero.receive(Potion::QuicksilverPotion), IsTrue());
      AssertThat(hero.hasRoomFor(Potion::QuicksilverPotion), IsTrue());
      AssertThat(hero.hasRoomFor(Potion::ReflexPotion), IsFalse());
    });
    it("shall consider all items as large for Rat Monarch", [] {
      Hero hero(HeroClass::RatMonarch);
      AssertThat(hero.receive(ShopItem::DragonSoul), IsTrue());
      AssertThat(hero.receive(Spell::Burndayraz), IsTrue());
      AssertThat(hero.receive(AlchemistSeal::CompressionSeal), IsTrue());
      AssertThat(hero.hasRoomFor(BlacksmithItem::BearMace), IsTrue());
      AssertThat(hero.hasRoomFor(Potion::HealthPotion), IsTrue());
      AssertThat(hero.receive(Potion::HealthPotion), IsTrue());
      AssertThat(hero.hasRoomFor(BlacksmithItem::BearMace), IsTrue());
      AssertThat(hero.receive(BlacksmithItem::BearMace), IsTrue());
      AssertThat(hero.hasRoomFor(BlacksmithItem::BearMace), IsFalse());
      AssertThat(hero.hasRoomFor(Potion::QuicksilverPotion), IsFalse());
      AssertThat(hero.hasRoomFor(Potion::HealthPotion), IsTrue());
    });
  });
  describe("Food", [] {
    it("shall be treated as a small inventory item", [] {
      Inventory inv{DungeonSetup{}};
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(27u));
    });
    it("shall be groupable", [] {
      Inventory inv{DungeonSetup{}};
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(inv.numFreeSmallSlots(), Equals(27u));
    });
    auto foodCount = [](const auto& inv) {
      const auto counts = inv.getItemCounts();
      const auto foodIt = std::find_if(begin(counts), end(counts),
                                       [](const auto& entry) { return entry.first == Item{MiscItem::Food}; });
      if (foodIt == end(counts))
        return 0u;
      return foodIt->second;
    };
    it("shall be counted correctly when adding, removing, clearing", [foodCount] {
      Inventory inv{DungeonSetup{}};
      AssertThat(foodCount(inv), Equals(0u));
      inv.remove(MiscItem::Food);
      AssertThat(foodCount(inv), Equals(0u));
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(foodCount(inv), Equals(1u));
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(foodCount(inv), Equals(2u));
      inv.remove(MiscItem::Food);
      AssertThat(foodCount(inv), Equals(1u));
      inv.clear();
      AssertThat(foodCount(inv), Equals(0u));
      inv.remove(MiscItem::Food);
      AssertThat(foodCount(inv), Equals(0u));
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(foodCount(inv), Equals(1u));
      AssertThat(inv.addFree(MiscItem::Food), IsTrue());
      AssertThat(foodCount(inv), Equals(2u));
      inv.clear();
      AssertThat(foodCount(inv), Equals(0u));
      AssertThat(inv.addFree(MiscItem::Food), IsTrue());
      AssertThat(foodCount(inv), Equals(1u));
    });
    it("shall have helper functions for adding, removing, counting", [foodCount] {
      Inventory inv{DungeonSetup{}};
      AssertThat(foodCount(inv), Equals(0u));
      AssertThat(inv.getFoodCount(), Equals(0u));

      AssertThat(inv.tryConsumeFood(1u), Equals(1u));
      AssertThat(foodCount(inv), Equals(0u));
      AssertThat(inv.getFoodCount(), Equals(0u));
      AssertThat(inv.addFood(1u), IsTrue());
      AssertThat(foodCount(inv), Equals(1u));
      AssertThat(inv.getFoodCount(), Equals(1u));
      AssertThat(inv.addFood(1u), IsTrue());
      AssertThat(foodCount(inv), Equals(2u));
      AssertThat(inv.getFoodCount(), Equals(2u));
      AssertThat(inv.tryConsumeFood(1u), Equals(0u));
      AssertThat(foodCount(inv), Equals(1u));
      AssertThat(inv.getFoodCount(), Equals(1u));

      inv.clear();
      AssertThat(foodCount(inv), Equals(0u));
      AssertThat(inv.getFoodCount(), Equals(0u));

      AssertThat(inv.addFood(123u), IsTrue());
      AssertThat(foodCount(inv), Equals(123u));
      AssertThat(inv.getFoodCount(), Equals(123u));
      inv.remove(MiscItem::Food);
      AssertThat(foodCount(inv), Equals(122u));
      AssertThat(inv.getFoodCount(), Equals(122u));
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(inv.tryConsumeFood(125u), Equals(2u));
      AssertThat(foodCount(inv), Equals(0u));
      AssertThat(inv.getFoodCount(), Equals(0u));
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(foodCount(inv), Equals(1u));
      AssertThat(inv.getFoodCount(), Equals(1u));
    });
    it("shall be grouped correctly", [] {
      Inventory inv{DungeonSetup{}};
      inv.remove(MiscItem::Food);
      AssertThat(inv.add(MiscItem::Food), IsTrue());
      AssertThat(inv.addFood(123u), IsTrue());
      inv.remove(MiscItem::Food);
      AssertThat(inv.tryConsumeFood(23u), Equals(0u));
      const auto counts = inv.getItemsGrouped();
      const auto foodIt = std::find_if(begin(counts), end(counts), [](const auto& entry) {
        return entry.first.itemOrSpell == ItemOrSpell{MiscItem::Food};
      });
      AssertThat(foodIt, !Equals(end(counts)));
      AssertThat(foodIt->second, Equals(100u));
    });
    it("shall be available initially for Herbivore", [] {
      Hero hero(HeroClass::Goatperson, {God::BinlorIronshield});
      const auto counts = hero.getItemCounts();
      const auto foodIt = std::find_if(begin(counts), end(counts),
                                       [](const auto& entry) { return entry.first == Item{MiscItem::Food}; });
      AssertThat(foodIt, !Equals(end(counts)));
      AssertThat(foodIt->second, Equals(90u));
    });
  });
}
