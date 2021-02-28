#include "bandit/bandit.h"

#include "Hero.hpp"
#include "Inventory.hpp"
#include "Items.hpp"
#include "Spells.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
}

void testInventory()
{
  describe("Inventory", [] {
    it("shall have free health and mana potion by default", [] {
      Inventory inv;
      AssertThat(inv.has(Item::HealthPotion), IsTrue());
      AssertThat(inv.has(Item::ManaPotion), IsTrue());
    });
    it("shall have room for 30 small / 6 large items", [] {
      Inventory inv;
      AssertThat(inv.numFreeSmallSlots(), Equals(28));
      inv.clear();
      AssertThat(inv.numFreeSmallSlots(), Equals(30));
      inv.add(Item::BadgeOfHonour);
      AssertThat(inv.numFreeSmallSlots(), Equals(25));
      inv.add(Item::BloodySigil);
      inv.add(Item::FineSword);
      inv.add(Item::PendantOfHealth);
      inv.add(Item::PendantOfMana);
      AssertThat(inv.numFreeSmallSlots(), Equals(5));
      AssertThat(inv.hasRoomFor(Item::TowerShield), IsTrue());
      inv.add(Item::Spoon);
      AssertThat(inv.numFreeSmallSlots(), Equals(4));
      AssertThat(inv.hasRoomFor(Item::MagePlate), IsFalse());
      AssertThat(inv.hasRoomFor(Item::TowerShield), IsFalse());
      AssertThat(inv.hasRoomFor(Item::DragonSoul), IsTrue());
      inv.add(Item::Spoon);
      inv.add(Item::HealthPotion);
      inv.add(Item::DragonSoul);
      inv.add(Item::CompressionSeal);
      AssertThat(inv.numFreeSmallSlots(), Equals(0));
    });
    it("shall account for grouping", [] {
      Inventory inv;
      inv.clear();
      inv.add(Item::HealthPotion);
      AssertThat(inv.numFreeSmallSlots(), Equals(29));
      inv.add(Item::HealthPotion);
      AssertThat(inv.numFreeSmallSlots(), Equals(29));
      inv.add(Item::HealthPotion);
      AssertThat(inv.numFreeSmallSlots(), Equals(29));
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
      AssertThat(hero.hasRoomFor(Item::KegOfMana), IsTrue());
      hero.receive(Item::KegOfMana);
      AssertThat(hero.hasRoomFor(Item::WickedGuitar), IsTrue());
      hero.receive(Item::WickedGuitar);
      AssertThat(hero.hasRoomFor(Item::OrbOfZot), IsFalse());
      AssertThat(hero.hasRoomFor(Item::QuicksilverPotion), IsTrue());
      hero.receive(Item::QuicksilverPotion);
      AssertThat(hero.hasRoomFor(Item::QuicksilverPotion), IsTrue());
      AssertThat(hero.hasRoomFor(Item::ReflexPotion), IsFalse());
    });
    it("shall consider all items as large for Rat Monarch", [] {
      Hero hero(HeroClass::RatMonarch);
      hero.receive(Item::DragonSoul);
      hero.receive(Spell::Burndayraz);
      hero.receive(Item::CompressionSeal);
      AssertThat(hero.hasRoomFor(Item::BearMace), IsTrue());
      AssertThat(hero.hasRoomFor(Item::HealthPotion), IsTrue());
      hero.receive(Item::HealthPotion);
      AssertThat(hero.hasRoomFor(Item::BearMace), IsTrue());
      hero.receive(Item::BearMace);
      AssertThat(hero.hasRoomFor(Item::BearMace), IsFalse());
      AssertThat(hero.hasRoomFor(Item::QuicksilverPotion), IsFalse());
      AssertThat(hero.hasRoomFor(Item::HealthPotion), IsTrue());
    });
  });
}
