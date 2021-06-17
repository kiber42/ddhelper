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
      inv.add(Potion::HealthPotion);
      AssertThat(inv.sellingPrice(Potion::HealthPotion), Equals(10));
      inv.add(Potion::ManaPotion);
      AssertThat(inv.sellingPrice(Potion::ManaPotion), Equals(10));
    });
    it("shall have room for 30 small / 6 large items", [&] {
      AssertThat(inv.numFreeSmallSlots(), Equals(28u));
      inv.clear();
      AssertThat(inv.numFreeSmallSlots(), Equals(30u));
      inv.add(ShopItem::BadgeOfHonour);
      AssertThat(inv.numFreeSmallSlots(), Equals(25u));
      inv.add(ShopItem::BloodySigil);
      inv.add(ShopItem::FineSword);
      inv.add(ShopItem::PendantOfHealth);
      inv.add(ShopItem::PendantOfMana);
      AssertThat(inv.numFreeSmallSlots(), Equals(5u));
      AssertThat(inv.hasRoomFor(ShopItem::TowerShield), IsTrue());
      inv.add(ShopItem::Spoon);
      AssertThat(inv.numFreeSmallSlots(), Equals(4u));
      AssertThat(inv.hasRoomFor(ShopItem::MagePlate), IsFalse());
      AssertThat(inv.hasRoomFor(ShopItem::TowerShield), IsFalse());
      AssertThat(inv.hasRoomFor(ShopItem::DragonSoul), IsTrue());
      inv.add(ShopItem::Spoon);
      inv.add(Potion::HealthPotion);
      inv.add(ShopItem::DragonSoul);
      inv.add(AlchemistSeal::CompressionSeal);
      AssertThat(inv.numFreeSmallSlots(), Equals(0u));
    });
    it("shall account for grouping", [&] {
      inv.clear();
      inv.add(Potion::HealthPotion);
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
      inv.add(Potion::HealthPotion);
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
      inv.add(Potion::HealthPotion);
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
      inv.addFree(Potion::HealthPotion);
      AssertThat(inv.numFreeSmallSlots(), Equals(29u));
    });
    it("shall treat spells as large for most classes", [] {
      Hero hero(HeroClass::Sorcerer);
      hero.receive(Spell::Apheelsik);
      hero.receive(Spell::Bludtupowa);
      hero.receiveFreeSpell(Spell::Burndayraz);
      hero.receive(Spell::Bysseps);
      AssertThat(hero.hasRoomFor(Spell::Cydstepp), IsTrue());
      hero.receive(Spell::Cydstepp);
      AssertThat(hero.hasRoomFor(Spell::Endiswal), IsFalse());
      AssertThat(hero.hasRoomFor(Spell::Cydstepp), IsFalse());
    });
    it("shall treat spells as small for Wizards", [] {
      Hero hero(HeroClass::Wizard);
      AssertThat(hero.has(Spell::Burndayraz), IsTrue());
      hero.receiveFreeSpell(Spell::Apheelsik);
      hero.receive(Spell::Apheelsik);
      hero.receive(Spell::Bludtupowa);
      hero.receiveFreeSpell(Spell::Burndayraz);
      hero.receive(Spell::Bysseps);
      hero.receive(Spell::Cydstepp);
      hero.receive(Spell::Endiswal);
      hero.receive(Spell::Getindare);
      hero.receive(Spell::Halpmeh);
      hero.receive(Spell::Imawal);
      hero.receive(Spell::Lemmisi);
      hero.receive(Spell::Pisorf);
      hero.receive(Spell::Weytwut);
      hero.receive(Spell::Wonafyt);
      hero.receiveFreeSpell(Spell::Wonafyt);
      hero.receive(Spell::Wonafyt);
      AssertThat(hero.getItemsAndSpells().size(), Equals(19u));
      AssertThat(hero.getSpells().size(), Equals(17u));
      AssertThat(hero.getItemCounts().size(), Equals(2u));
      const auto spellCounts = hero.getSpellCounts();
      AssertThat(spellCounts.size(), Equals((unsigned)Spell::Last + 1));
      AssertThat(spellCounts.back().first, Equals(Spell::Wonafyt));
      AssertThat(spellCounts.back().second, Equals(3));
      AssertThat(hero.hasRoomFor(ShopItem::KegOfMana), IsTrue());
      hero.receive(ShopItem::KegOfMana);
      AssertThat(hero.hasRoomFor(ShopItem::WickedGuitar), IsTrue());
      hero.receive(ShopItem::WickedGuitar);
      AssertThat(hero.hasRoomFor(ShopItem::OrbOfZot), IsFalse());
      AssertThat(hero.hasRoomFor(Potion::QuicksilverPotion), IsTrue());
      hero.receive(Potion::QuicksilverPotion);
      AssertThat(hero.hasRoomFor(Potion::QuicksilverPotion), IsTrue());
      AssertThat(hero.hasRoomFor(Potion::ReflexPotion), IsFalse());
    });
    it("shall consider all items as large for Rat Monarch", [] {
      Hero hero(HeroClass::RatMonarch);
      hero.receive(ShopItem::DragonSoul);
      hero.receive(Spell::Burndayraz);
      hero.receive(AlchemistSeal::CompressionSeal);
      AssertThat(hero.hasRoomFor(BlacksmithItem::BearMace), IsTrue());
      AssertThat(hero.hasRoomFor(Potion::HealthPotion), IsTrue());
      hero.receive(Potion::HealthPotion);
      AssertThat(hero.hasRoomFor(BlacksmithItem::BearMace), IsTrue());
      hero.receive(BlacksmithItem::BearMace);
      AssertThat(hero.hasRoomFor(BlacksmithItem::BearMace), IsFalse());
      AssertThat(hero.hasRoomFor(Potion::QuicksilverPotion), IsFalse());
      AssertThat(hero.hasRoomFor(Potion::HealthPotion), IsTrue());
    });
  });
}
