#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/Defence.hpp"
#include "engine/Hero.hpp"
#include "engine/Magic.hpp"
#include "engine/Monster.hpp"
#include "engine/Resources.hpp"

#include <cstdint>

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
  SimpleResources resources;

  auto attack(Hero& hero, Monster& monster) { return Combat::attack(hero, monster, noOtherMonsters, resources); }

  void cast(Hero& hero, Spell spell) { Magic::cast(hero, spell, noOtherMonsters, resources); }

  void cast(Hero& hero, Monster& monster, Spell spell) { Magic::cast(hero, monster, spell, noOtherMonsters, resources); }
} // namespace

void testConversionBonusesBasic()
{
  describe("Human", [] {
    it("should receive one base damage for 100 conversion points", [] {
      auto human = Hero{HeroClass::Guard, HeroRace::Human};
      human.addConversionPoints(99, noOtherMonsters);
      AssertThat(human.getDamageBonusPercent(), Equals(0));
      human.addConversionPoints(1, noOtherMonsters);
      AssertThat(human.getDamageBonusPercent(), Equals(10));
      human.addConversionPoints(100, noOtherMonsters);
      AssertThat(human.getDamageBonusPercent(), Equals(20));
    });
  });
  describe("Elf", [] {
    it("should receive one max mana point for 70 conversion points", [] {
      auto elf = Hero{HeroClass::Guard, HeroRace::Elf};
      elf.addConversionPoints(69, noOtherMonsters);
      AssertThat(elf.getManaPointsMax(), Equals(10u));
      elf.addConversionPoints(1, noOtherMonsters);
      AssertThat(elf.getManaPointsMax(), Equals(11u));
      AssertThat(elf.getManaPoints(), Equals(10u));
    });
  });
  describe("Dwarf", [] {
    it("should receive one health bonus for 80 conversion points", [] {
      auto dwarf = Hero{HeroClass::Guard, HeroRace::Dwarf};
      dwarf.addConversionPoints(79, noOtherMonsters);
      dwarf.gainLevel(noOtherMonsters);
      dwarf.gainLevel(noOtherMonsters);
      AssertThat(dwarf.getHitPointsMax(), Equals(30u));
      dwarf.addConversionPoints(1, noOtherMonsters);
      AssertThat(dwarf.getHitPointsMax(), Equals(33u));
      AssertThat(dwarf.getHitPoints(), Equals(30u));
      dwarf.gainLevel(noOtherMonsters);
      AssertThat(dwarf.getHitPointsMax(), Equals(44u));
    });
  });
  auto itemCount = [](const Hero& hero, Item itemType) {
    const auto& inventory = hero.getItemsAndSpells();
    return std::count_if(begin(inventory), end(inventory), [&](auto& entry) {
      if (auto item = std::get_if<Item>(&entry.itemOrSpell))
        return *item == itemType;
      return false;
    });
  };
  describe("Halfling", [&] {
    it("should receive one health potion for 80 conversion points", [&] {
      auto halfling = Hero{HeroClass::Guard, HeroRace::Halfling};
      halfling.addConversionPoints(79, noOtherMonsters);
      AssertThat(itemCount(halfling, Potion::HealthPotion), Equals(1u));
      halfling.addConversionPoints(1, noOtherMonsters);
      AssertThat(itemCount(halfling, Potion::HealthPotion), Equals(2u));
    });
  });
  describe("Gnome", [&] {
    it("should receive one mana potion for 90 conversion points", [&] {
      auto gnome = Hero{HeroClass::Guard, HeroRace::Gnome};
      gnome.addConversionPoints(89, noOtherMonsters);
      AssertThat(itemCount(gnome, Potion::ManaPotion), Equals(1u));
      gnome.addConversionPoints(1, noOtherMonsters);
      AssertThat(itemCount(gnome, Potion::ManaPotion), Equals(2u));
    });
  });
  describe("Orc", [] {
    it("should receive two base damage for 80 conversion points", [] {
      auto orc = Hero{HeroClass::Guard, HeroRace::Orc};
      orc.addConversionPoints(79, noOtherMonsters);
      AssertThat(orc.getBaseDamage(), Equals(5u));
      orc.addConversionPoints(1, noOtherMonsters);
      AssertThat(orc.getBaseDamage(), Equals(7u));
    });
  });
  describe("Goblin", [] {
    it("should receive stacking XP bonus for 85 conversion points", [] {
      auto goblin = Hero{HeroClass::Guard, HeroRace::Goblin};
      goblin.addConversionPoints(84, noOtherMonsters);
      AssertThat(goblin.getLevel(), Equals(1u));
      AssertThat(goblin.getXP(), Equals(0u));
      unsigned expected = 5u - goblin.getXPforNextLevel();
      goblin.addConversionPoints(1, noOtherMonsters);
      AssertThat(goblin.getLevel(), Equals(2u));
      AssertThat(goblin.getXP(), Equals(expected));
      expected += 6u;
      goblin.addConversionPoints(85, noOtherMonsters);
      AssertThat(goblin.getLevel(), Equals(2u));
      AssertThat(goblin.getXP(), Equals(expected));
      expected += 7u - goblin.getXPforNextLevel();
      goblin.addConversionPoints(85, noOtherMonsters);
      AssertThat(goblin.getLevel(), Equals(3u));
      AssertThat(goblin.getXP(), Equals(expected));
    });
  });
}

void testConversionBonusesMonsterClasses()
{
  describe("Vampire", [] {
    it("should receive one level of lifesteal for 120 conversion points", [] {
      auto vampire = Hero{HeroClass::Vampire};
      vampire.addConversionPoints(119, noOtherMonsters);
      AssertThat(vampire.getIntensity(HeroStatus::LifeSteal), Equals(1u));
      vampire.addConversionPoints(1, noOtherMonsters);
      AssertThat(vampire.getIntensity(HeroStatus::LifeSteal), Equals(2u));
    });
  });
  describe("Half-Dragon", [] {
    it("should receive 20% knockback for 100 conversion points", [] {
      auto halfDragon = Hero{HeroClass::HalfDragon};
      halfDragon.addConversionPoints(99, noOtherMonsters);
      AssertThat(halfDragon.getIntensity(HeroStatus::Knockback), Equals(20u));
      halfDragon.addConversionPoints(1, noOtherMonsters);
      AssertThat(halfDragon.getIntensity(HeroStatus::Knockback), Equals(40u));
    });
  });
  describe("Gorgon", [] {
    it("should receive 5% death gaze for 100 conversion points", [] {
      auto gorgon = Hero{HeroClass::Gorgon};
      gorgon.addConversionPoints(99, noOtherMonsters);
      AssertThat(gorgon.getIntensity(HeroStatus::DeathGaze), Equals(10u));
      gorgon.addConversionPoints(1, noOtherMonsters);
      AssertThat(gorgon.getIntensity(HeroStatus::DeathGaze), Equals(15u));
    });
  });
  describe("Rat Monarch", [] {
    it("should receive one level of corrosive strike for 80 conversion points", [] {
      auto ratMonarch = Hero{HeroClass::RatMonarch};
      ratMonarch.addConversionPoints(79, noOtherMonsters);
      AssertThat(ratMonarch.getIntensity(HeroStatus::CorrosiveStrike), Equals(1u));
      ratMonarch.addConversionPoints(1, noOtherMonsters);
      AssertThat(ratMonarch.getIntensity(HeroStatus::CorrosiveStrike), Equals(2u));
    });
  });
  describe("Goatperson", [] {
    it("should receive HP and MP refresh as conversion bonus", [] {
      auto altars = std::vector{God::BinlorIronshield};
      auto goatperson = Hero{HeroClass::Goatperson, altars};
      auto boss = Monster{MonsterType::DoomArmour, Level{10}};
      goatperson.add(HeroStatus::DeathProtection);
      AssertThat(attack(goatperson, boss), Equals(Summary::Safe));
      AssertThat(goatperson.getHitPoints(), Equals(1u));
      goatperson.addConversionPoints(99, noOtherMonsters);
      AssertThat(goatperson.getHitPoints(), Equals(1u));
      goatperson.addConversionPoints(1, noOtherMonsters);
      AssertThat(goatperson.getHitPoints(), Equals(10u));

      goatperson.add(HeroStatus::DeathProtection);
      cast(goatperson, boss, Spell::Pisorf);
      cast(goatperson, boss, Spell::Apheelsik);
      AssertThat(attack(goatperson, boss), Equals(Summary::Safe));
      AssertThat(goatperson.getHitPoints(), Equals(1u));
      AssertThat(goatperson.getManaPoints(), Equals(1u));
      goatperson.addConversionPoints(100, noOtherMonsters);
      AssertThat(goatperson.getHitPoints(), Equals(1u));
      AssertThat(goatperson.getManaPoints(), Equals(1u));
      goatperson.addConversionPoints(10, noOtherMonsters);
      AssertThat(goatperson.getHitPoints(), Equals(10u));
      AssertThat(goatperson.getManaPoints(), Equals(10u));

      cast(goatperson, Spell::Getindare);
      cast(goatperson, boss, Spell::Burndayraz);
      AssertThat(goatperson.getManaPoints(), Equals(1u));
      goatperson.addConversionPoints(110, noOtherMonsters);
      AssertThat(goatperson.getManaPoints(), Equals(1u));
      goatperson.addConversionPoints(10, noOtherMonsters);
      AssertThat(goatperson.getManaPoints(), Equals(10u));
    });
  });
}

void testChemist()
{
  describe("Additives", [] {
    Hero chemist(HeroClass::Chemist);
    it("should allow to stack Might from Bysseps and make Might increase base damage", [&] {
      cast(chemist, Spell::Bysseps);
      cast(chemist, Spell::Bysseps);
      chemist.recover(4u, noOtherMonsters);
      cast(chemist, Spell::Bysseps);
      AssertThat(chemist.getIntensity(HeroStatus::Might), Equals(3u));
      AssertThat(chemist.getBaseDamage(), Equals(8u));
      AssertThat(chemist.getDamageBonusPercent(), Equals(90));
    });
    it("should double the spell costs for Bysseps", [&] {
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(16u));
      AssertThat(chemist.getManaPoints(), Equals(0u));
    });
    it("should not stack erode effect", [&] {
      Monster monster("", {Level{1}, 16_HP, 1_damage}, {4_physicalresist, 2_magicalresist}, {});
      AssertThat(attack(chemist, monster), Equals(Summary::Safe));
      AssertThat(monster.getHitPoints(), Equals(1u));
      AssertThat(monster.getPhysicalResistPercent(), Equals(1u));
      AssertThat(monster.getMagicalResistPercent(), Equals(0u));
    });
    it("should wear off after one attack", [&] {
      AssertThat(chemist.has(HeroStatus::Might), IsFalse());
      AssertThat(chemist.getBaseDamage(), Equals(5u));
      AssertThat(chemist.getDamageBonusPercent(), Equals(0));
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(2u));
    });
    it("should silently stack on Might received by other means", [&] {
      chemist.add(HeroStatus::Might);
      chemist.recover(10u, noOtherMonsters);
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(2u));
      cast(chemist, Spell::Bysseps);
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(4u));
      cast(chemist, Spell::Bysseps);
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(8u));
      AssertThat(chemist.getIntensity(HeroStatus::Might), Equals(3u));
      AssertThat(chemist.getBaseDamage(), Equals(8u));
      AssertThat(chemist.getDamageBonusPercent(), Equals(90));
    });
    it("should not allow stacking Might by other means than Bysseps", [] {
      Hero chemist(HeroClass::Chemist);
      cast(chemist, Spell::Bysseps);
      AssertThat(chemist.getIntensity(HeroStatus::Might), Equals(1u));
      chemist.followDeity(God::BinlorIronshield, 1000, resources);
      resources.numWalls = 10;
      chemist.request(Boon::StoneForm, noOtherMonsters, resources);
      chemist.wallDestroyed();
      AssertThat(chemist.getIntensity(HeroStatus::Might), Equals(1u));
      Monster monster("", {Level{1}, 16_HP, 1_damage}, {4_physicalresist, 2_magicalresist}, {});
      attack(chemist, monster);
      AssertThat(chemist.has(HeroStatus::Might), IsFalse());

      chemist.wallDestroyed();
      AssertThat(chemist.getIntensity(HeroStatus::Might), Equals(1u));
      cast(chemist, Spell::Bysseps);
      AssertThat(chemist.getIntensity(HeroStatus::Might), Equals(2u));
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(4u));
    });
  });
}

void testTransmuter()
{
  describe("Spirit Sword", [] {
    it("should apply spirit strength on conversion", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      const auto baseDamage = hero.getBaseDamage();
      hero.add(HeroTrait::SpiritSword);
      hero.addConversionPoints(1000, noOtherMonsters);
      const auto spiritStrength = hero.getLevel() + hero.getManaPointsMax();
      AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(spiritStrength));
      AssertThat(hero.getManaPoints(), Equals(0));
      AssertThat(hero.getBaseDamage(), Equals(baseDamage + spiritStrength));
    });
    it("should work correctly for Transmuter Elves", [] {
      // Elf transmuter conversion threshold is a special case:
      // the freshly added mana point is counted for spirit strength
      Hero hero(HeroClass::Transmuter, HeroRace::Elf);
      AssertThat(hero.getManaPoints(), Equals(10));
      hero.addConversionPoints(70, noOtherMonsters);
      AssertThat(hero.getManaPointsMax(), Equals(11));
      AssertThat(hero.getManaPoints(), Equals(0));
      AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(hero.getLevel() + hero.getManaPointsMax()));
    });
  });
}

void testHeroTraits()
{
  testConversionBonusesBasic();
  testConversionBonusesMonsterClasses();
  testChemist();
  testTransmuter();
}

void testMonsterTraits()
{
  describe("Berserk", [] {
    it("should become active when threshold is reached", [] {
      for (uint8_t level = 1u; level <= 10u; ++level)
      {
        Monster minotaur(MonsterType::Minotaur, Level{level});
        AssertThat(minotaur.getBerserkPercent(), Equals(50u));
        AssertThat(minotaur.isEnraged(), IsFalse());
        const auto damage50 = minotaur.getHitPointsMax() - minotaur.getHitPoints() / 2;
        minotaur.takeDamage(damage50 - 1, DamageType::Typeless);
        AssertThat(minotaur.isEnraged(), IsFalse());
        minotaur.takeDamage(1, DamageType::Typeless);
        AssertThat(minotaur.isEnraged(), IsTrue());
        minotaur.recover(1);
        AssertThat(minotaur.isEnraged(), IsFalse());
      }
    });
  });
}

void testTraits()
{
  testHeroTraits();
  testMonsterTraits();
}