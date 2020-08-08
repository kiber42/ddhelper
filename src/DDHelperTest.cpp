#include "bandit/bandit.h"

#include "Dungeon.hpp"
#include "Experience.hpp"
#include "Hero.hpp"
#include "Melee.hpp"
#include "Monster.hpp"
#include "MonsterFactory.hpp"

using namespace bandit;
using namespace snowhouse;

go_bandit([] {
  describe("Hero's level", [] {
    Hero hero;
    it("should initially be 1", [&] { AssertThat(hero.getLevel(), Equals(1)); });
    it("should be 2 after receiving 5 XP", [&] {
      hero.gainExperience(5);
      AssertThat(hero.getLevel(), Equals(2));
    });
    it("should be 3 after gaining another level", [&] {
      hero.gainLevel();
      AssertThat(hero.getLevel(), Equals(3));
    });
    it("should never exceed 10 (prestige awarded instead)", [&] {
      hero.gainExperience(1000);
      AssertThat(hero.getLevel(), Equals(10));
      AssertThat(hero.getPrestige(), Equals(10));
    });
    it("should be 9 after receiving a modifier of -1", [&] {
      hero.modifyLevelBy(-1);
      AssertThat(hero.getLevel(), Equals(9));
      AssertThat(hero.getPrestige(), Equals(10));
    });
    it("should never be below 1", [&] {
      hero.modifyLevelBy(-10);
      AssertThat(hero.getLevel(), Equals(1));
    });
    it("should never be above 10", [&] {
      hero.modifyLevelBy(+10);
      AssertThat(hero.getLevel(), Equals(10));
    });
  });

  describe("Level up (from XP)", [] {
    Hero hero;
    hero.gainLevel();
    hero.loseHitPointsOutsideOfFight(4);
    hero.gainExperience(10);
    it("should refill hit points", [&] { AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax())); });
    it("should increase maximum hit points", [&] { AssertThat(hero.getHitPointsMax(), Equals(30)); });
  });

  describe("Level up (from item or boon)", [] {
    Hero hero;
    hero.gainExperience(5);
    hero.loseHitPointsOutsideOfFight(1);
    hero.gainLevel();
    it("should refill hit points", [&] { AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax())); });
    it("should increase maximum hit points", [&] { AssertThat(hero.getHitPointsMax(), Equals(30)); });
  });

  describe("Hero's XP", [] {
    it("should be 2 instead of 1 if the hero has 'Learning' (permanent bonus)", [&] {
      Hero hero;
      hero.addStatus(HeroStatus::Learning);
      hero.gainExperience(1);
      AssertThat(hero.getXP(), Equals(2));
      AssertThat(hero.hasStatus(HeroStatus::Learning), Equals(true));
    });
    it("should grow by 1 extra point for each level of 'Learning'", [&] {
      Hero hero;
      hero.addStatus(HeroStatus::Learning);
      hero.addStatus(HeroStatus::Learning);
      hero.addStatus(HeroStatus::Learning);
      AssertThat(hero.getStatusIntensity(HeroStatus::Learning), Equals(3));
      hero.gainExperience(2);
      AssertThat(hero.getXP(), Equals(0));
      AssertThat(hero.getLevel(), Equals(2));
    });
    it("should grow by a bonus 50% (rounded down) if the hero has 'Experience Boost' (one time bonus, cannot stack)",
       [&] {
         Hero hero;
         hero.addStatus(HeroStatus::ExperienceBoost);
         AssertThat(hero.hasStatus(HeroStatus::ExperienceBoost), Equals(true));
         hero.gainExperience(2);
         AssertThat(hero.getXP(), Equals(3));
         AssertThat(hero.hasStatus(HeroStatus::ExperienceBoost), Equals(false));
         hero.addStatus(HeroStatus::ExperienceBoost);
         hero.addStatus(HeroStatus::ExperienceBoost);
         AssertThat(hero.getStatusIntensity(HeroStatus::ExperienceBoost), Equals(1));
         hero.gainExperience(1);
         AssertThat(hero.getXP(), Equals(4));
       });
    it("should first consider 'Experience Boost', then 'Learning'", [&] {
      Hero hero;
      // Start at level 2, otherwise 5 XP trigger a level up
      hero.gainLevel();
      hero.addStatus(HeroStatus::ExperienceBoost);
      hero.addStatus(HeroStatus::Learning);
      hero.gainExperience(3);
      // 150% * 3 + 1 = 5.5 -> 5 rather than 150% * (3 + 1) = 6
      AssertThat(hero.getXP(), Equals(5));
    });

    // TO DO: Further traits that affect XP
    // - Alchemist's Pact (gain 3 XP on consuming potion)
    // - Veteran (Fighter trait, 1 bonus XP per kill)
  });

  describe("Monster", [] {
    Monster monster(makeGenericMonsterStats(2, 10, 3), Defence(), MonsterTraits());
    it("used for test should have level 2 and 10 HP", [&] {
      AssertThat(monster.getLevel(), Equals(2));
      AssertThat(monster.getHitPoints(), Equals(10));
    });
    it("with 10 HP should survive a hit with 9 damage points and has 1 HP remaining", [&] {
      monster.takeDamage(9, false);
      AssertThat(monster.isDefeated(), Equals(false));
      AssertThat(monster.getHitPoints(), Equals(1));
    });
    it("at level 2 should recover at a rate of 2 HP per explored square", [&] {
      monster.recover(4);
      AssertThat(monster.getHitPoints(), Equals(9));
    });
    it("should not recover beyond its max HP", [&] {
      monster.recover(10);
      AssertThat(monster.getHitPoints(), Equals(monster.getHitPointsMax()));
    });
    it("should not recover HP while poisoned", [&] {
      monster.takeDamage(1, false);
      monster.poison(3);
      monster.recover(1);
      AssertThat(monster.getHitPoints(), Equals(9));
      AssertThat(monster.getPoisonAmount(), Equals(1));
    });
    it("should reduce poison as it would usually recover HP", [&] {
      monster.recover(1);
      AssertThat(monster.getHitPoints(), Equals(10));
      AssertThat(monster.isPoisoned(), Equals(false));
    });
    it("should lose 4 HP per caster level when hit by a fireball", [&] {
      monster.takeFireballDamage(2, false);
      AssertThat(monster.getHitPoints(), Equals(10 - 2 * 4));
    });
    it("should be burning after hit by a fireball", [&] {
      AssertThat(monster.isBurning(), Equals(true));
      // TODO: Special case: Wizard caster, burn stack size grows by 2 per fireball
      AssertThat(monster.getBurnStackSize(), Equals(1));
    });
    it("should recover HP at a rate reduced by 1 when burning", [&] {
      monster.recover(4);
      AssertThat(monster.getHitPoints(), Equals(6));
    });
    it("should take additional fireball damage when already burning", [&] {
      monster.takeFireballDamage(1, false);
      AssertThat(monster.getHitPoints(), Equals(6 - 1 * 4 - 1));
      AssertThat(monster.getBurnStackSize(), Equals(2));
    });
    it("should recover HP at a rate reduced by 1 when burning, independent of burn stack size", [&] {
      monster.recover(5);
      AssertThat(monster.getHitPoints(), Equals(6));
    });
    it("should take additional fireball damage per burn stack", [&] {
      monster.recover(10);
      monster.takeFireballDamage(1, false);
      AssertThat(monster.getHitPoints(), Equals(10 - 1 * 4 - 2));
    });
    it("should not have a burn stack size higher than twice the caster's level",
       [&] { AssertThat(monster.getBurnStackSize(), Equals(2)); });
    it("should stop burning upon any physical damage, and take damage equal to burn stack size", [&] {
      AssertThat(monster.getHitPoints() - monster.getBurnStackSize(), Equals(2));
      monster.takeDamage(0, false);
      AssertThat(monster.isBurning(), Equals(false));
      AssertThat(monster.getHitPoints(), Equals(2));
    });
    it("should recover from stun when taking damage", [&] {
      monster.stun();
      AssertThat(monster.isStunned(), Equals(true));
      monster.takeDamage(1, false);
      AssertThat(monster.isStunned(), Equals(false));
    });
  });

  describe("Monster damage", [] {
    Monster monster(makeGenericMonsterStats(3, 30, 10, 1), Defence(50, 75), MonsterTraits());
    it("should be adjusted for physical resistance", [&] {
      AssertThat(monster.getHitPoints(), Equals(30));
      AssertThat(monster.getPhysicalResistPercent(), Equals(50));
      monster.takeDamage(5 * 100 / (100 - 50), false);
      AssertThat(monster.getHitPoints(), Equals(30 - 5));
    });
    it("should be adjusted for physical resistance, resisted damage is rounded down", [&] {
      monster.takeDamage(1, false);
      AssertThat(monster.getHitPoints(), Equals(24));
    });
    it("should be adjusted for magical damage, resisted damage is rounded down", [&] {
      AssertThat(monster.getMagicalResistPercent(), Equals(75));
      monster.takeDamage(5 * 100 / (100 - 75) + 1, true);
      AssertThat(monster.getHitPoints(), Equals(24 - 5 - 1));
    });
    it("should be increased by corrosion", [&] {
      AssertThat(monster.getHitPoints(), Equals(18));
      monster.corrode();
      monster.takeDamage(10, false);
      AssertThat(monster.getHitPoints(), Equals(18 - 5 - 1));
    });
    it("should be increased by corrosion (2 levels -> 2 extra damage)", [&] {
      AssertThat(monster.getHitPoints(), Equals(12));
      monster.corrode();
      monster.takeDamage(20, true);
      AssertThat(monster.getHitPoints(), Equals(12 - 5 - 2));
    });
    it("should work for crushed resistances (3 percentage points lost per crushing)", [&] {
      monster.crushResistances();
      AssertThat(monster.getPhysicalResistPercent(), Equals(47));
      AssertThat(monster.getMagicalResistPercent(), Equals(72));
      monster.crushResistances();
      monster.crushResistances();
      monster.crushResistances();
      monster.crushResistances();
      AssertThat(monster.getPhysicalResistPercent(), Equals(47 - 12));
      AssertThat(monster.getMagicalResistPercent(), Equals(72 - 12));
      AssertThat(monster.getHitPoints(), Equals(5));
      monster.takeDamage(2, false);
      // 35% resistance remaining -> no damage reduction
      AssertThat(monster.getHitPoints() + monster.getCorroded(), Equals(3));
    });
    it("should account for death protection and have it wear off", [&] {
      AssertThat(monster.getDeathProtection(), Equals(1));
      monster.takeDamage(100, false);
      AssertThat(monster.isDefeated(), Equals(false));
      AssertThat(monster.getHitPoints(), Equals(1));
      AssertThat(monster.getDeathProtection(), Equals(0));
      monster.takeDamage(1, false);
      AssertThat(monster.isDefeated(), Equals(true));
    });
  });

  describe("Melee outcome prediction", [] {
    Hero hero;
    Monster monster(makeGenericMonsterStats(3, 15, 5), {}, {});
    Melee melee(hero, monster);
    it("should work for outcome 'safe' (simple case)",
       [&] { AssertThat(melee.predictOutcome().summary, Equals(Outcome::Summary::Safe)); });
    it("should work for outcome 'hero dies' (simple case)", [&] {
      hero.loseHitPointsOutsideOfFight(5);
      AssertThat(melee.predictOutcome().summary, Equals(Outcome::Summary::HeroDefeated));
    });
    it("should work for outcome 'hero wins' (one shot, monster has lower level)", [&] {
      hero.gainExperience(30);
      hero.loseHitPointsOutsideOfFight(hero.getHitPointsMax() - 1);
      AssertThat(melee.predictOutcome().summary, Equals(Outcome::Summary::HeroWins));
    });

    it("of hitpoint loss should work", [] {
      Hero hero;
      Monster monster(makeGenericMonsterStats(3, 15, 9), {}, {});
      Melee melee(hero, monster);
      const auto outcome = melee.predictOutcome();
      AssertThat(hero.getHitPoints(), Equals(10));
      AssertThat(monster.getHitPoints(), Equals(15));
      AssertThat(outcome.hero->getHitPoints(), Equals(10 - monster.getDamage()));
      AssertThat(outcome.monster->getHitPoints(), Equals(15 - hero.getDamage()));
    });
  });

  describe("Dungeon", [] {
    Dungeon dungeon(3, 3);
    auto monster = std::make_shared<Monster>(makeGenericMonsterStats(3, 30, 10, 0), Defence(), MonsterTraits());
    auto monster2 = std::make_shared<Monster>(*monster);
    it("should allow adding monsters if there is space", [&] {
      dungeon.add(monster, dungeon.randomFreePosition().value());
      AssertThat(dungeon.getMonsters().size(), Equals(1));
      for (unsigned n = 2; n <= 9; ++n)
      {
        AssertThat(dungeon.randomFreePosition().has_value(), Equals(true));
        dungeon.add(monster2, dungeon.randomFreePosition().value());
        AssertThat(dungeon.getMonsters().size(), Equals(n));
      }
      AssertThat(dungeon.randomFreePosition().has_value(), Equals(false));
      AssertThat(dungeon.isFree({0, 0}), Equals(false));
    });
    it("should not be revealed initially", [&] { AssertThat(dungeon.isRevealed({1, 1}), Equals(false)); });
    it("should consider a square with a defeated monster as free", [&] {
      monster->takeDamage(100, false);
      AssertThat(monster->isDefeated(), Equals(true));
      dungeon.update();
      AssertThat(dungeon.randomFreePosition().has_value(), Equals(true));
    });
    auto hero = std::make_shared<Hero>();
    it("should consider the hero's square as occupied", [&] {
      dungeon.setHero(hero, dungeon.randomFreePosition().value());
      AssertThat(dungeon.randomFreePosition().has_value(), Equals(false));
    });
    it("should be revealed around the hero's position", [&] { AssertThat(dungeon.isRevealed({1, 1}), Equals(true)); });
  });

  describe("Pathfinding", [] {
    Dungeon dungeon(10, 10);
    auto hero = std::make_shared<Hero>();
    dungeon.setHero(hero, Position(0, 0));
    it("should find paths if there are no obstacles", [&] { AssertThat(dungeon.isConnected({9, 9}), Equals(true)); });
    it("should consider squares inaccessible if there are no revealed paths to them", [&] {
      AssertThat(dungeon.isAccessible({9, 9}), Equals(false));
    });
    it("should consider squares accessible if there is a revealed path", [&] {
      for (int x = 2; x < 10; ++x)
        dungeon.reveal({x, x});
      AssertThat(dungeon.isAccessible({9, 9}), Equals(true));
    });
  });
});

// Missing:
// - exploration
// - path finding
// - find possible actions
// - representations
// - spells
// - inventory

int main(int argc, char* argv[])
{
  // Run the tests
  return bandit::run(argc, argv);
}
