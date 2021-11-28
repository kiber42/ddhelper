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
} // namespace

void testRaces()
{
  describe("Elf", [] {
    it("should receive one max mana point on conversion", [] {
      Hero hero(HeroClass::Guard, HeroRace::Elf);
      hero.addConversionPoints(70, noOtherMonsters);
      AssertThat(hero.getManaPointsMax(), Equals(11));
      AssertThat(hero.getManaPoints(), Equals(10));
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
  testRaces();
  testChemist();
  testTransmuter();
}

void testMonsterTraits()
{
  describe("Berserk", [] {
    it("should become active when threshold is reached", [] {
      for (uint8_t level = 1u; level <= 10u; ++level)
      {
        Monster minotaur(MonsterType::Minotaur, level);
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