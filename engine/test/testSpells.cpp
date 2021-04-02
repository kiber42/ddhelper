#include "bandit/bandit.h"

#include "Hero.hpp"
#include "Monster.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
  SimpleResources resources;
} // namespace

void testBludtupowa()
{
  describe("Bludtupowa", [] {
    it("should have costs based on the hero's level", [] {
      Hero hero;
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(3));
      hero.gainLevel(noOtherMonsters);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(6));
      hero.gainLevel(noOtherMonsters);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(9));
    });
    it("should convert health to MP", [] {
      Hero hero;
      hero.loseManaPoints(10);
      Magic::cast(hero, Spell::Bludtupowa, noOtherMonsters, resources);
      AssertThat(hero.getHitPoints(), Equals(7));
      AssertThat(hero.getManaPoints(), Equals(3));
      Magic::cast(hero, Spell::Bludtupowa, noOtherMonsters, resources);
      AssertThat(hero.getHitPoints(), Equals(4));
      AssertThat(hero.getManaPoints(), Equals(6));
    });
    it("should uncover tiles", [] {
      Hero hero;
      SimpleResources resources;
      resources.numHiddenTiles = 4;
      Magic::cast(hero, Spell::Bludtupowa, noOtherMonsters, resources);
      AssertThat(resources.numHiddenTiles, Equals(1));
      hero.loseManaPoints(10);
      Magic::cast(hero, Spell::Bludtupowa, noOtherMonsters, resources);
      AssertThat(hero.getHitPoints(), Equals(4));
      AssertThat(hero.getManaPoints(), Equals(1));
      Magic::cast(hero, Spell::Bludtupowa, noOtherMonsters, resources);
      AssertThat(hero.getHitPoints(), Equals(1));
      AssertThat(hero.getManaPoints(), Equals(1));
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
      hero.addStatus(HeroStatus::DeathProtection);
      hero.loseHitPointsOutsideOfFight(hero.getHitPoints() - Magic::healthCostsBludtupowa(hero), noOtherMonsters);
      AssertThat(Magic::isPossible(hero, Spell::Bludtupowa, resources), IsFalse());
    });
    it("should cost 4 HP more for sorcerers", [] {
      Hero hero(HeroClass::Sorcerer);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(7));
      hero.gainLevel(noOtherMonsters);
      AssertThat(Magic::healthCostsBludtupowa(hero), Equals(10));
      Magic::cast(hero, Spell::Bludtupowa, noOtherMonsters, resources);
      AssertThat(hero.getHitPoints(), Equals(10));
    });
  });
}

void testSpells()
{
  testBludtupowa();
}
