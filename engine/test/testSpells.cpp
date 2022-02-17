#include "bandit/bandit.h"

#include "engine/Hero.hpp"
#include "engine/Magic.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
  SimpleResources resources;

  void cast(Hero& hero, Spell spell) { Magic::cast(hero, spell, noOtherMonsters, resources); }

  auto cast(Hero& hero, Monster& monster, Spell spell)
  {
    return Magic::cast(hero, monster, spell, noOtherMonsters, resources);
  }
} // namespace

void testBludtupowa()
{
  describe("Bludtupowa", [] {
    it("should have costs based on the hero's level", [] {
      Hero hero;
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(3u));
      hero.gainLevel(noOtherMonsters);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(6u));
      hero.gainLevel(noOtherMonsters);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(9u));
    });
    it("should convert health to MP", [] {
      Hero hero;
      hero.loseManaPoints(10);
      cast(hero, Spell::Bludtupowa);
      AssertThat(hero.getHitPoints(), Equals(7u));
      AssertThat(hero.getManaPoints(), Equals(3u));
      cast(hero, Spell::Bludtupowa);
      AssertThat(hero.getHitPoints(), Equals(4u));
      AssertThat(hero.getManaPoints(), Equals(6u));
    });
    it("should uncover tiles", [] {
      Hero hero;
      resources.numHiddenTiles = 4;
      cast(hero, Spell::Bludtupowa);
      AssertThat(resources.numHiddenTiles, Equals(1u));
      hero.loseManaPoints(10);
      cast(hero, Spell::Bludtupowa);
      AssertThat(hero.getHitPoints(), Equals(4u));
      AssertThat(hero.getManaPoints(), Equals(1u));
      cast(hero, Spell::Bludtupowa);
      AssertThat(hero.getHitPoints(), Equals(1u));
      AssertThat(hero.getManaPoints(), Equals(1u));
    });
    it("should be available only if the hero has sufficient health", [] {
      Hero hero;
      AssertThat(Magic::isPossible(hero, Spell::Bludtupowa, resources), IsTrue());
      hero.loseHitPointsOutsideOfFight(7, noOtherMonsters);
      AssertThat(Magic::isPossible(hero, Spell::Bludtupowa, resources), IsFalse());
      hero.gainLevel(noOtherMonsters);
      AssertThat(Magic::isPossible(hero, Spell::Bludtupowa, resources), IsTrue());
      hero.loseHitPointsOutsideOfFight(13, noOtherMonsters);
      AssertThat(Magic::isPossible(hero, Spell::Bludtupowa, resources), IsTrue());
      hero.loseHitPointsOutsideOfFight(1, noOtherMonsters);
      AssertThat(Magic::isPossible(hero, Spell::Bludtupowa, resources), IsFalse());
    });
    it("should be independent of death protection", [] {
      Hero hero;
      hero.add(HeroStatus::DeathProtection);
      hero.loseHitPointsOutsideOfFight(hero.getHitPoints() - Magic::healthCostsBludtupowa(hero), noOtherMonsters);
      AssertThat(Magic::isPossible(hero, Spell::Bludtupowa, resources), IsFalse());
    });
    it("should cost 4 HP more for sorcerers", [] {
      Hero hero(HeroClass::Sorcerer);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(7u));
      hero.gainLevel(noOtherMonsters);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(10u));
      cast(hero, Spell::Bludtupowa);
      AssertThat(hero.getHitPoints(), Equals(10));
    });
  });
}

void testBurndayraz()
{
  describe("Burndayraz", [] {
    // TODO
  });
  describe("Retaliation", [] {
    it("shall be triggered", [] {
      Hero hero;
      Monster djinn{MonsterType::Djinn, Level{2}};
      AssertThat(cast(hero, djinn, Spell::Burndayraz), Equals(Summary::Safe));
      AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() - djinn.getDamage() / 2));
    });
    it("shall trigger side effects", [] {
      Hero hero;
      Monster illusion{MonsterType::Illusion, Level{2}};
      AssertThat(cast(hero, illusion, Spell::Burndayraz), Equals(Summary::Safe));
      AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() - illusion.getDamage() / 2));
      AssertThat(hero.getIntensity(HeroDebuff::Weakened), Equals(1u));
    });
  });
  describe("Burndown", [] {
    it("shall deal damage equal to burn stack size", [] {
      Hero hero;
      Monster monster{MonsterType::MeatMan, Level{2}};
      cast(hero, monster, Spell::Burndayraz);
      hero.recoverManaPoints(6);
      cast(hero, monster, Spell::Burndayraz);
      AssertThat(monster.isBurning(), IsTrue());
      AssertThat(monster.getBurnStackSize(), Equals(2u));
      monster.recover(100);
      monster.burnDown();
      AssertThat(monster.getHitPointsMax() - monster.getHitPoints(), Equals(2u));
      AssertThat(monster.getBurnStackSize(), Equals(0u));
      monster.burnMax(4);
      monster.burnDown();
      AssertThat(monster.getHitPointsMax() - monster.getHitPoints(), Equals(6u));
      monster.burnMax(monster.getHitPoints());
      monster.burnDown();
      AssertThat(monster.isDefeated(), IsTrue());
    });
    it("shall account for magical resist", [] {
      Hero hero;
      auto monster = Monster{{Level{1}, 10_HP, 0_damage}, {20_magicalresist}};
      monster.burnMax(11);
      monster.burnDown();
      AssertThat(monster.getHitPoints(), Equals(1u /* 10 - (11 - 11 * 2 / 10) */));
    });
  });
}

void testSpells()
{
  testBludtupowa();
  testBurndayraz();
}
