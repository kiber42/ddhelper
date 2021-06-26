#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/Hero.hpp"
#include "engine/Magic.hpp"
#include "engine/Monster.hpp"
#include "engine/Resources.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
  SimpleResources resources;
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
      Magic::cast(chemist, Spell::Bysseps, noOtherMonsters, resources);
      Magic::cast(chemist, Spell::Bysseps, noOtherMonsters, resources);
      chemist.recover(4u);
      Magic::cast(chemist, Spell::Bysseps, noOtherMonsters, resources);
      AssertThat(chemist.getStatusIntensity(HeroStatus::Might), Equals(3u));
      AssertThat(chemist.getBaseDamage(), Equals(8u));
      AssertThat(chemist.getDamageBonusPercent(), Equals(90));
    });
    it("should double the spell costs for Bysseps", [&] {
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(16u));
      AssertThat(chemist.getManaPoints(), Equals(0u));
    });
    it("should not stack erode effect", [&] {
      Monster monster("", {Level{1}, 16_HP, 1_damage}, {4, 2}, {});
      AssertThat(Combat::attack(chemist, monster, noOtherMonsters), Equals(Summary::Safe));
      AssertThat(monster.getHitPoints(), Equals(1u));
      AssertThat(monster.getPhysicalResistPercent(), Equals(1u));
      AssertThat(monster.getMagicalResistPercent(), Equals(0u));
    });
    it("should wear off after one attack", [&] {
      AssertThat(chemist.hasStatus(HeroStatus::Might), IsFalse());
      AssertThat(chemist.getBaseDamage(), Equals(5u));
      AssertThat(chemist.getDamageBonusPercent(), Equals(0));
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(2u));
    });
    it("should silently stack on Might received by other means", [&]{
      chemist.addStatus(HeroStatus::Might);
      chemist.recover(10u);
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(2u));
      Magic::cast(chemist, Spell::Bysseps, noOtherMonsters, resources);
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(4u));
      Magic::cast(chemist, Spell::Bysseps, noOtherMonsters, resources);
      AssertThat(Magic::spellCosts(Spell::Bysseps, chemist), Equals(8u));
      AssertThat(chemist.getStatusIntensity(HeroStatus::Might), Equals(3u));
      AssertThat(chemist.getBaseDamage(), Equals(8u));
      AssertThat(chemist.getDamageBonusPercent(), Equals(90));
    });
    it("should not allow stacking Might by other means than Bysseps", []{
      Hero chemist(HeroClass::Chemist);
      Magic::cast(chemist, Spell::Bysseps, noOtherMonsters, resources);
      AssertThat(chemist.getStatusIntensity(HeroStatus::Might), Equals(1u));
      chemist.followDeity(God::BinlorIronshield, 1000);
      resources.numWalls = 10;
      chemist.request(Boon::StoneForm, noOtherMonsters, resources);
      chemist.wallDestroyed();
      AssertThat(chemist.getStatusIntensity(HeroStatus::Might), Equals(1u));
      Monster monster("", {Level{1}, 16_HP, 1_damage}, {4, 2}, {});
      Combat::attack(chemist, monster, noOtherMonsters);
      AssertThat(chemist.hasStatus(HeroStatus::Might), IsFalse());

      chemist.wallDestroyed();
      AssertThat(chemist.getStatusIntensity(HeroStatus::Might), Equals(1u));
      Magic::cast(chemist, Spell::Bysseps, noOtherMonsters, resources);
      AssertThat(chemist.getStatusIntensity(HeroStatus::Might), Equals(2u));
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
      hero.addTrait(HeroTrait::SpiritSword);
      hero.addConversionPoints(1000, noOtherMonsters);
      const auto spiritStrength = hero.getLevel() + hero.getManaPointsMax();
      AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(spiritStrength));
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
      AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength),
                 Equals(hero.getLevel() + hero.getManaPointsMax()));
    });
  });
}

void testHeroTraits()
{
  testRaces();
  testChemist();
  testTransmuter();
}
