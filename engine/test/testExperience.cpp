#include "bandit/bandit.h"

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
}

void testHeroExperience()
{
  describe("Hero's level", [] {
    Hero hero;
    it("should initially be 1", [&] { AssertThat(hero.getLevel(), Equals(1u)); });
    it("should be 2 after receiving 5 XP", [&] {
      hero.gainExperienceNoBonuses(5, noOtherMonsters);
      AssertThat(hero.getLevel(), Equals(2u));
    });
    it("should be 3 after gaining another level", [&] {
      hero.gainLevel(noOtherMonsters);
      AssertThat(hero.getLevel(), Equals(3u));
    });
    it("should never exceed 10 (prestige awarded instead)", [&] {
      hero.gainExperienceNoBonuses(1000, noOtherMonsters);
      AssertThat(hero.getLevel(), Equals(10u));
      AssertThat(hero.getPrestige(), Equals(10u));
    });
    it("should be 9 after receiving a modifier of -1", [&] {
      hero.modifyLevelBy(-1);
      AssertThat(hero.getLevel(), Equals(9u));
      AssertThat(hero.getPrestige(), Equals(10u));
    });
    it("should never be below 1", [&] {
      hero.modifyLevelBy(-10);
      AssertThat(hero.getLevel(), Equals(1u));
    });
    it("should never be above 10", [&] {
      hero.modifyLevelBy(+10);
      AssertThat(hero.getLevel(), Equals(10u));
    });
  });

  describe("Level up (from XP)", [] {
    Hero hero;
    hero.gainLevel(noOtherMonsters);
    hero.loseHitPointsOutsideOfFight(4, noOtherMonsters);
    hero.gainExperienceNoBonuses(10, noOtherMonsters);
    it("should refill hit points", [&] { AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax())); });
    it("should increase maximum hit points", [&] { AssertThat(hero.getHitPointsMax(), Equals(30u)); });
    hero.addStatus(HeroDebuff::Poisoned, noOtherMonsters);
    hero.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
    hero.gainExperienceNoBonuses(15, noOtherMonsters);
    it("should remove poison and mana burn", [&] {
      AssertThat(hero.hasStatus(HeroDebuff::Poisoned), IsFalse());
      AssertThat(hero.hasStatus(HeroDebuff::ManaBurned), IsFalse());
    });
  });

  describe("Level up (from item or boon)", [] {
    Hero hero;
    hero.gainExperienceNoBonuses(5, noOtherMonsters);
    hero.loseHitPointsOutsideOfFight(1, noOtherMonsters);
    hero.gainLevel(noOtherMonsters);
    it("should refill hit points", [&] { AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax())); });
    it("should increase maximum hit points", [&] { AssertThat(hero.getHitPointsMax(), Equals(30u)); });
    hero.addStatus(HeroDebuff::Poisoned, noOtherMonsters);
    hero.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
    hero.gainLevel(noOtherMonsters);
    it("should remove poison and mana burn", [&] {
      AssertThat(hero.hasStatus(HeroDebuff::Poisoned), IsFalse());
      AssertThat(hero.hasStatus(HeroDebuff::ManaBurned), IsFalse());
    });
  });

  describe("Hero's XP", [] {
    it("should be 2 instead of 1 if the hero has 'Learning' (permanent bonus)", [&] {
      Hero hero;
      hero.addStatus(HeroStatus::Learning);
      hero.gainExperienceForKill(1, false, noOtherMonsters);
      AssertThat(hero.getXP(), Equals(2u));
      AssertThat(hero.hasStatus(HeroStatus::Learning), IsTrue());
    });
    it("should grow by 1 extra point for each level of 'Learning'", [&] {
      Hero hero;
      hero.addStatus(HeroStatus::Learning);
      hero.addStatus(HeroStatus::Learning);
      hero.addStatus(HeroStatus::Learning);
      hero.addStatus(HeroStatus::Learning);
      AssertThat(hero.getStatusIntensity(HeroStatus::Learning), Equals(4u));
      hero.gainExperienceForKill(1, false, noOtherMonsters);
      AssertThat(hero.getXP(), Equals(0u));
      AssertThat(hero.getLevel(), Equals(2u));
    });
    it("should grow by a bonus 50% (rounded down) if the hero has 'Experience Boost' (one time bonus, cannot stack)",
       [&] {
         Hero hero;
         hero.gainLevel(noOtherMonsters);
         hero.addStatus(HeroStatus::ExperienceBoost);
         AssertThat(hero.hasStatus(HeroStatus::ExperienceBoost), IsTrue());
         hero.gainExperienceForKill(2, false, noOtherMonsters);
         AssertThat(hero.getXP(), Equals(3u));
         AssertThat(hero.hasStatus(HeroStatus::ExperienceBoost), IsFalse());
         hero.addStatus(HeroStatus::ExperienceBoost);
         hero.addStatus(HeroStatus::ExperienceBoost);
         AssertThat(hero.getStatusIntensity(HeroStatus::ExperienceBoost), Equals(1u));
         hero.gainExperienceForKill(1, false, noOtherMonsters);
         AssertThat(hero.getXP(), Equals(4u));
       });
    it("should first consider 'Experience Boost', then 'Learning'", [&] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      hero.gainLevel(noOtherMonsters);
      hero.addStatus(HeroStatus::ExperienceBoost);
      hero.addStatus(HeroStatus::Learning);
      hero.gainExperienceForKill(3, false, noOtherMonsters);
      // 150% * 3 + 1 = 5.5 -> 5 rather than 150% * (3 + 1) = 6
      AssertThat(hero.getXP(), Equals(5u));
    });

    // TODO: Further traits that affect XP
    // - Alchemist's Pact (gain 3 XP on consuming potion)
    // - Veteran (Fighter trait, 1 bonus XP per kill, different level thresholds)
  });
}
