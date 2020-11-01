#include "bandit/bandit.h"

#include "Combat.hpp"
#include "Dungeon.hpp"
#include "Hero.hpp"
#include "Items.hpp"
#include "Monster.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

using namespace bandit;
using namespace snowhouse;

void testHeroExperience()
{
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
    hero.addStatus(HeroStatus::Poisoned);
    hero.addStatus(HeroStatus::ManaBurned);
    hero.gainExperience(15);
    it("should remove poison and mana burn", [&] {
      AssertThat(hero.hasStatus(HeroStatus::Poisoned), IsFalse());
      AssertThat(hero.hasStatus(HeroStatus::ManaBurned), IsFalse());
    });
  });

  describe("Level up (from item or boon)", [] {
    Hero hero;
    hero.gainExperience(5);
    hero.loseHitPointsOutsideOfFight(1);
    hero.gainLevel();
    it("should refill hit points", [&] { AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax())); });
    it("should increase maximum hit points", [&] { AssertThat(hero.getHitPointsMax(), Equals(30)); });
    hero.addStatus(HeroStatus::Poisoned);
    hero.addStatus(HeroStatus::ManaBurned);
    hero.gainLevel();
    it("should remove poison and mana burn", [&] {
      AssertThat(hero.hasStatus(HeroStatus::Poisoned), IsFalse());
      AssertThat(hero.hasStatus(HeroStatus::ManaBurned), IsFalse());
    });
  });

  describe("Hero's XP", [] {
    it("should be 2 instead of 1 if the hero has 'Learning' (permanent bonus)", [&] {
      Hero hero;
      hero.addStatus(HeroStatus::Learning);
      hero.gainExperience(1);
      AssertThat(hero.getXP(), Equals(2));
      AssertThat(hero.hasStatus(HeroStatus::Learning), IsTrue());
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
         AssertThat(hero.hasStatus(HeroStatus::ExperienceBoost), IsTrue());
         hero.gainExperience(2);
         AssertThat(hero.getXP(), Equals(3));
         AssertThat(hero.hasStatus(HeroStatus::ExperienceBoost), IsFalse());
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
    // - Veteran (Fighter trait, 1 bonus XP per kill, different level thresholds)
  });
}

void testMonsterBasics()
{
  describe("Monster", [] {
    Monster monster(2, 10, 3);
    it("used for test should have level 2 and 10 HP", [&] {
      AssertThat(monster.getLevel(), Equals(2));
      AssertThat(monster.getHitPoints(), Equals(10));
    });
    it("with 10 HP should survive a hit with 9 damage points and has 1 HP remaining", [&] {
      monster.takeDamage(9, false);
      AssertThat(monster.isDefeated(), IsFalse());
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
      AssertThat(monster.isPoisoned(), IsFalse());
    });
    it("should lose 4 HP per caster level when hit by a fireball", [&] {
      monster.takeFireballDamage(2);
      AssertThat(monster.getHitPoints(), Equals(10 - 2 * 4));
    });
    it("should be burning after hit by a fireball", [&] {
      AssertThat(monster.isBurning(), IsTrue());
      // TODO: Special case: Wizard caster, burn stack size grows by 2 per fireball
      AssertThat(monster.getBurnStackSize(), Equals(1));
    });
    it("should recover HP at a rate reduced by 1 when burning", [&] {
      monster.recover(4);
      AssertThat(monster.getHitPoints(), Equals(6));
    });
    it("should take additional fireball damage when already burning", [&] {
      monster.takeFireballDamage(1);
      AssertThat(monster.getHitPoints(), Equals(6 - 1 * 4 - 1));
      AssertThat(monster.getBurnStackSize(), Equals(2));
    });
    it("should recover HP at a rate reduced by 1 when burning, independent of burn stack size", [&] {
      monster.recover(5);
      AssertThat(monster.getHitPoints(), Equals(6));
    });
    it("should take additional fireball damage per burn stack", [&] {
      monster.recover(10);
      monster.takeFireballDamage(1);
      AssertThat(monster.getHitPoints(), Equals(10 - 1 * 4 - 2));
    });
    it("should not have a burn stack size higher than twice the caster's level",
       [&] { AssertThat(monster.getBurnStackSize(), Equals(2)); });
    it("should stop burning upon any physical damage, and take damage equal to burn stack size", [&] {
      AssertThat(monster.getHitPoints() - monster.getBurnStackSize(), Equals(2));
      monster.takeDamage(0, false);
      AssertThat(monster.isBurning(), IsFalse());
      AssertThat(monster.getHitPoints(), Equals(2));
    });
    it("should recover from being slowed when taking damage", [&] {
      monster.slow();
      AssertThat(monster.isSlowed(), IsTrue());
      monster.takeDamage(1, false);
      AssertThat(monster.isSlowed(), IsFalse());
    });
  });
}

void testDefenceBasics()
{
  describe("Monster", [] {
    describe("Physical resistance", [] {
      Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{50, 0}, {});
      it("should reduce damage according to resistance %", [&] {
        AssertThat(monster.getPhysicalResistPercent(), Equals(50));
        monster.takeDamage(10, false);
        AssertThat(monster.getHitPoints(), Equals(5));
      });
      it("should be rounded down", [&] {
        monster.takeDamage(1, false);
        AssertThat(monster.getHitPoints(), Equals(4));
      });
    });
    describe("Magical resistance", [] {
      Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{0, 75}, {});
      it("should be adjusted for magical damage, resisted damage is rounded down", [&] {
        AssertThat(monster.getMagicalResistPercent(), Equals(75));
        monster.takeDamage(5 * 4 + 1, true);
        AssertThat(monster.getHitPoints(), Equals(10 - 5 - 1));
      });
    });
    describe("Corrosion", [] {
      it("should add 1 damage point per level", [] {
        Hero hero;
        Monster monster(1, 20, 1);
        monster.corrode();
        monster.takeDamage(1, false);
        AssertThat(monster.getHitPoints(), Equals(18));
        monster.corrode();
        monster.takeDamage(1, true);
        AssertThat(monster.getHitPoints(), Equals(15));
      });
    });
    describe("Resistance crushing", [] {
      it("should reduce resistances by 3 percentage points", [&] {
        Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{50, 75}, {});
        monster.erodeResitances();
        AssertThat(monster.getPhysicalResistPercent(), Equals(47));
        AssertThat(monster.getMagicalResistPercent(), Equals(72));
        monster.erodeResitances();
        monster.erodeResitances();
        monster.erodeResitances();
        monster.erodeResitances();
        AssertThat(monster.getPhysicalResistPercent(), Equals(47 - 12));
        AssertThat(monster.getMagicalResistPercent(), Equals(72 - 12));
        // 35% physical resistance remaining -> no damage reduction
        monster.takeDamage(2, false);
        AssertThat(monster.getHitPoints(), Equals(8));
      });
    });
    describe("Death protection", [] {
      Monster monster("", MonsterStats{1, 10, 10, 2}, {}, {});
      it("should prevent defeat", [&] {
        monster.takeDamage(100, false);
        AssertThat(monster.isDefeated(), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1));
      });
      it("should wear off", [&] {
        monster.takeDamage(100, false);
        AssertThat(monster.isDefeated(), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1));
        AssertThat(monster.getDeathProtection(), Equals(0));
        monster.takeDamage(1, false);
        AssertThat(monster.isDefeated(), IsTrue());
      });
    });
  });

  describe("Hero", [] {
    describe("Physical resistance", [] {
      Hero hero;
      hero.setPhysicalResistPercent(50);
      it("should be accounted for (rounding down)", [&] {
        hero.takeDamage(11, false);
        AssertThat(hero.getHitPoints(), Equals(10 - 6));
      });
      it("should not block magical damage", [&] {
        AssertThat(hero.getHitPoints(), Equals(4));
        hero.takeDamage(3, true);
        AssertThat(hero.getHitPoints(), Equals(1));
      });
      it("should be capped (at 65% by default)", [&] {
        hero.setPhysicalResistPercent(100);
        hero.changePhysicalResistPercent(+100);
        AssertThat(hero.getPhysicalResistPercent(), Equals(65));
      });
    });
    describe("Magical resistance", [] {
      Hero hero;
      hero.setMagicalResistPercent(50);
      it("should be accounted for (rounding down)", [&] {
        hero.takeDamage(11, true);
        AssertThat(hero.getHitPoints(), Equals(10 - 6));
      });
      it("should not block physical damage", [&] {
        AssertThat(hero.getHitPoints(), Equals(4));
        hero.takeDamage(3, false);
        AssertThat(hero.getHitPoints(), Equals(1));
      });
      it("should be capped (at 65% by default)", [&] {
        hero.setMagicalResistPercent(100);
        hero.changeMagicalResistPercent(+100);
        AssertThat(hero.getMagicalResistPercent(), Equals(65));
      });
    });
    describe("Damage reduction", [] {
      it("should reduce incoming damage", [&] {
        Hero hero;
        hero.addStatus(HeroStatus::DamageReduction);
        hero.takeDamage(3, false);
        AssertThat(hero.getHitPoints(), Equals(8));
        hero.addStatus(HeroStatus::DamageReduction, 99);
        hero.takeDamage(90, false);
        hero.takeDamage(99, true);
        AssertThat(hero.getHitPoints(), Equals(8));
        hero.takeDamage(104, false);
        AssertThat(hero.getHitPoints(), Equals(4));
        hero.addStatus(HeroStatus::DeathProtection);
        hero.takeDamage(104, true);
        AssertThat(hero.getHitPoints(), Equals(1));
        AssertThat(hero.hasStatus(HeroStatus::DeathProtection), IsFalse());
        hero.takeDamage(101, true);
        AssertThat(hero.isDefeated(), IsTrue());
      });
    });
    describe("Corrosion", [] {
      it("should increase damage taken by 1 per stack size", [] {
        Hero hero;
        hero.addStatus(HeroStatus::Corrosion);
        hero.takeDamage(1, false);
        AssertThat(hero.getHitPoints(), Equals(8));
        hero.takeDamage(1, true);
        AssertThat(hero.getHitPoints(), Equals(6));
        hero.addStatus(HeroStatus::Corrosion);
        hero.addStatus(HeroStatus::Corrosion);
        hero.takeDamage(3, true);
        AssertThat(hero.isDefeated(), IsTrue());
      });
      it("should only cause extra damage if any damage was taken at all", [] {
        Hero hero;
        hero.addStatus(HeroStatus::Corrosion);
        hero.addStatus(HeroStatus::DamageReduction);
        hero.takeDamage(1, false);
        AssertThat(hero.getHitPoints(), Equals(10));
      });
    });
    describe("Damage reduction, resistance and corrosion", [] {
      it("should be applied in this order", [] {
        Hero hero;
        hero.setPhysicalResistPercent(50);
        hero.addStatus(HeroStatus::DamageReduction, 2);
        hero.addStatus(HeroStatus::Corrosion, 5);
        hero.takeDamage(3, false);
        const int expectedDamage = (3 - 2) - 0 /* 50% of 1, rounded down */ + 5;
        AssertThat(hero.getHitPoints(), Equals(10 - expectedDamage));
      });
    });
  });
}

void testMelee()
{
  describe("Melee simulation", [] {
    Hero hero;
    Monster monster(3, 15, 5);
    it("should correctly predict safe outcome (simple case)",
       [&] { AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe)); });
    it("should correctly predict death outcome (simple case)",
       [&] { AssertThat(Combat::attack(hero, monster), Equals(Summary::Death)); });
    it("should correctly predict win outcome (one shot, monster has lower level)", [&] {
      Hero hero;
      hero.gainExperience(30);
      AssertThat(hero.getLevel(), Equals(4));
      AssertThat(hero.getXP(), Equals(0));
      hero.loseHitPointsOutsideOfFight(hero.getHitPointsMax() - 1);
      monster.recover(100);
      AssertThat(Combat::attack(hero, monster), Equals(Summary::Win));
    });
    it("should correctly predict hitpoint loss (simple case)", [] {
      Hero hero;
      Monster monster(3, 15, 9);
      AssertThat(hero.getHitPoints(), Equals(10));
      AssertThat(monster.getHitPoints(), Equals(15));
      Combat::attack(hero, monster);
      AssertThat(hero.getHitPoints(), Equals(10 - monster.getDamage()));
      AssertThat(monster.getHitPoints(), Equals(15 - hero.getDamageOutputVersus(monster)));
    });
  });

  describe("Monster death gaze", [] {
    it("should petrify hero with low health (<50%)", [] {
      Hero hero;
      Monster gorgon(MonsterType::Gorgon, 1);
      AssertThat(Combat::attack(hero, gorgon), Equals(Summary::Win));
      hero.loseHitPointsOutsideOfFight(3);
      AssertThat(hero.getHitPoints(), Equals(4));
      Monster gorgon2(MonsterType::Gorgon, 1);
      AssertThat(Combat::attack(hero, gorgon2), Equals(Summary::Petrified));
    });
    it("should be available with 100% intensity", [] {
      Hero hero;
      auto traits = MonsterTraitsBuilder().setDeathGazePercent(100).get();
      Monster monster("", {1, 100, 1, 0}, {}, std::move(traits));
      hero.addStatus(HeroStatus::DeathProtection);
      AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
      AssertThat(hero.hasStatus(HeroStatus::DeathProtection), IsTrue());
      hero.recover(100);
      hero.loseHitPointsOutsideOfFight(1);
      AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
      AssertThat(hero.hasStatus(HeroStatus::DeathProtection), IsFalse());
      hero.recover(100);
      hero.loseHitPointsOutsideOfFight(1);
      AssertThat(Combat::attack(hero, monster), Equals(Summary::Petrified));
    });
    it("should be available with 101% intensity", [] {
      Hero hero;
      auto traits = MonsterTraitsBuilder().setDeathGazePercent(101).get();
      Monster monster("", {1, 10, 1, 0}, {}, std::move(traits));
      AssertThat(Combat::attack(hero, monster), Equals(Summary::Petrified));
    });
  });
}

void testStatusEffects()
{
  describe("Status", [] {
    describe("Burning Strike", [] { /* TODO */ });
    describe("Consecrated Strike", [] {
      Hero hero;
      it("should result in magical damage", [&] {
        AssertThat(hero.doesMagicalDamage(), IsFalse());
        hero.addStatus(HeroStatus::ConsecratedStrike);
        AssertThat(hero.doesMagicalDamage(), IsTrue());
      });
      it("should wear off", [&] {
        Monster monster("", MonsterStats{1, 5, 1, 0}, Defence{100, 0}, {});
        Combat::attack(hero, monster);
        AssertThat(monster.isDefeated(), IsTrue());
        AssertThat(hero.doesMagicalDamage(), IsFalse());
        AssertThat(hero.hasStatus(HeroStatus::ConsecratedStrike), IsFalse());
      });
    });
    describe("Corrosive Strike", [] {
      Hero hero;
      hero.gainLevel();
      hero.addStatus(HeroStatus::CorrosiveStrike);
      it("should corrode monster", [&] {
        Monster monster(MonsterType::MeatMan, 2);
        Combat::attack(hero, monster);
        AssertThat(monster.getCorroded(), Equals(1));
        hero.addStatus(HeroStatus::FirstStrike);
        hero.addStatus(HeroStatus::CorrosiveStrike, 2);
        hero.fullHealthAndMana();
        Combat::attack(hero, monster);
        AssertThat(monster.getCorroded(), Equals(4));
        AssertThat(monster.getHitPointsMax() - monster.getHitPoints(), Equals(2 * hero.getDamageVersusStandard() + 1));
      });
    });
    describe("Crushing Blow", [] {
      Hero hero;
      before_each([&] {
        hero.addStatus(HeroStatus::CrushingBlow);
        hero.addStatus(HeroStatus::DeathProtection);
      });
      it("should reduce the monster's health to 75%", [&] {
        Monster monster(MonsterType::MeatMan, 10);
        Combat::attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(monster.getHitPointsMax() * 3 / 4));
      });
      it("should ignore physical immunity", [&] {
        Monster monster("", MonsterStats{1, 100, 1, 0}, Defence{100, 100}, {});
        Combat::attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(75));
      });
      it("should ignore magical immunity", [&] {
        Monster monster("", MonsterStats{1, 100, 1, 0}, Defence{100, 100}, {});
        hero.addStatus(HeroStatus::MagicalAttack);
        Combat::attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(75));
      });
      it("should not affect monsters below 75% health", [&] {
        Monster monster(MonsterType::MeatMan, 1);
        monster.takeDamage(monster.getHitPoints() - 1, false);
        Combat::attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(1));
      });
      it("should wear off", [&] {
        Monster monster(MonsterType::MeatMan, 10);
        Combat::attack(hero, monster);
        AssertThat(hero.hasStatus(HeroStatus::CrushingBlow), IsFalse());
      });
    });
    describe("Cursed", [] {
      it("should negate resistances", [] {
        Hero hero;
        hero.setPhysicalResistPercent(50);
        hero.takeDamage(4, false);
        AssertThat(hero.getHitPoints(), Equals(10 - 4 / 2));
        hero.addStatus(HeroStatus::Cursed);
        hero.takeDamage(4, false);
        AssertThat(hero.getHitPoints(), Equals(10 - 4 / 2 - 4));
      });
      it("should be added/removed when cursed/not-cursed monster is defeated", [] {
        Hero hero;
        hero.changeBaseDamage(100);
        Monster monster("", MonsterStats{1, 10, 3, 0}, {}, std::move(MonsterTraitsBuilder().addCurse()));
        Combat::attack(hero, monster);
        // One curse from hit, one from killing
        AssertThat(hero.getStatusIntensity(HeroStatus::Cursed), Equals(2));
        Monster monster2(1, 10, 3);
        Combat::attack(hero, monster2);
        AssertThat(hero.getStatusIntensity(HeroStatus::Cursed), Equals(1));
      });
      it("should take effect immediately after hero's attack", [] {
        Hero hero;
        hero.gainLevel();
        const int health = hero.getHitPoints();
        hero.setPhysicalResistPercent(50);
        Monster monster("", MonsterStats(1, 100, 10, 0), {}, std::move(MonsterTraitsBuilder().addCurse()));
        Combat::attack(hero, monster);
        AssertThat(hero.hasStatus(HeroStatus::Cursed), IsTrue());
        AssertThat(hero.getHitPoints(), Equals(health - 10));
      });
    });
    describe("Curse Immune", [] {
      it("should prevent being cursed", [] {
        Hero hero;
        hero.addStatus(HeroStatus::CurseImmune);
        hero.addStatus(HeroStatus::Cursed);
        AssertThat(hero.hasStatus(HeroStatus::Cursed), Equals(false));
        Hero hero2;
        hero2.addStatus(HeroStatus::Cursed);
        AssertThat(hero2.hasStatus(HeroStatus::Cursed), Equals(true));
        hero2.addStatus(HeroStatus::CurseImmune);
        AssertThat(hero2.hasStatus(HeroStatus::Cursed), Equals(false));
      });
    });
    describe("Death Gaze", [] { /* TODO */ });
    describe("Death Gaze Immune", [] {
      it("should prevent petrification", [] {
        Hero hero;
        hero.takeDamage(6, false);
        hero.addStatus(HeroStatus::DeathGazeImmune);
        Monster gorgon(MonsterType::Gorgon, 1);
        AssertThat(Combat::attack(hero, gorgon), Equals(Summary::Win));
      });
    });
    describe("Dodge", [] {
      it("should result in hero not taking hit", [] {
        Hero hero;
        hero.addStatus(HeroStatus::DodgeTemporary, 100);
        Monster boss(MonsterType::Gorgon, 10);
        AssertThat(hero.getDodgeChancePercent(), Equals(100));
        AssertThat(Combat::attack(hero, boss), Equals(Summary::Safe));
        AssertThat(hero.getDodgeChancePercent(), Equals(0));
        hero.addStatus(HeroStatus::DodgePermanent, 100);
        AssertThat(hero.getDodgeChancePercent(), Equals(100));
        AssertThat(Combat::attack(hero, boss), Equals(Summary::Safe));
        AssertThat(hero.getDodgeChancePercent(), Equals(100));
      });
      it("should succeed with given probability", [] {
        Hero hero;
        for (int chance = 10; chance <= 100; chance += 10)
        {
          hero.addStatus(HeroStatus::DodgePrediction);
          hero.addDodgeChancePercent(10, false);
          int attempts = 1;
          while (!hero.predictDodgeNext() && attempts < 100)
          {
            ++attempts;
            hero.addStatus(HeroStatus::DodgeTemporary, 0); // trigger reroll
          }
          const int tolerance = 2 * (11 - chance / 10);
          const int expectedWithTolerance = 100 / chance * tolerance;
          AssertThat(attempts, IsLessThan(expectedWithTolerance));
        }
      });
      it("chance should be rerolled after each attack", [] {
        Hero hero;
        hero.addDodgeChancePercent(50, true);
        hero.addStatus(HeroStatus::DodgePrediction);
        hero.gainLevel();
        bool dodgeNext = hero.predictDodgeNext();
        int changes = 0;
        for (int i = 0; i < 1000; ++i)
        {
          // Alternate between level 1 and 2 monsters to change attack order
          Monster monster(MonsterType::MeatMan, 1 + i % 2);
          Combat::attack(hero, monster);
          if (dodgeNext)
          {
            AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax()));
            AssertThat(hero.hasStatus(HeroStatus::DodgePrediction), IsFalse());
            hero.addStatus(HeroStatus::DodgePrediction);
          }
          else
          {
            AssertThat(hero.getHitPoints(), IsLessThan(hero.getHitPointsMax()));
            AssertThat(hero.hasStatus(HeroStatus::DodgePrediction), IsTrue());
            hero.fullHealthAndMana();
          }
          if (dodgeNext != hero.predictDodgeNext())
          {
            ++changes;
            dodgeNext = !dodgeNext;
          }
        }
        // should change every other time (on average)
        AssertThat(changes, IsGreaterThan(400));
      });
      it("should not interfere with reflexes", [] {
        Hero hero;
        hero.addStatus(HeroStatus::DodgeTemporary, 100);
        hero.addStatus(HeroStatus::Reflexes);
        Monster monster(MonsterType::MeatMan, 2);
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
        AssertThat(monster.getHitPointsMax() - monster.getHitPoints(), Equals(2 * hero.getDamageVersusStandard()));
      });
    });
    describe("Exhausted", [] {
      Hero hero;
      hero.gainLevel();
      hero.gainLevel();
      hero.addTrait(HeroTrait::Damned);
      it("should be computed automatically for damned hero", [&] {
        AssertThat(hero.hasStatus(HeroStatus::Exhausted), IsFalse());
        hero.loseHitPointsOutsideOfFight(3);
        AssertThat(hero.hasStatus(HeroStatus::Exhausted), IsTrue());
      });
      it("should limit health recovery", [&] {
        AssertThat(hero.getHitPoints(), Equals(27));
        Combat::uncoverTiles(hero, nullptr, 1);
        AssertThat(hero.getHitPoints(), Equals(28));
      });
      it("should prevent mana recovery", [&] {
        hero.loseManaPoints(1);
        AssertThat(hero.getManaPoints(), Equals(9));
        Combat::uncoverTiles(hero, nullptr, 2);
        AssertThat(hero.getManaPoints(), Equals(9));
        AssertThat(hero.hasStatus(HeroStatus::Exhausted), IsFalse());
        Combat::uncoverTiles(hero, nullptr, 1);
        AssertThat(hero.getManaPoints(), Equals(10));
      });
    });
    describe("First Strike", [] {
      it("should give initiative versus regular monsters of any level", [&] {
        Hero hero;
        Monster boss(MonsterType::Goat, 10);
        hero.addStatus(HeroStatus::FirstStrike);
        AssertThat(hero.hasInitiativeVersus(boss), IsTrue());
      });
      it("should not give initiative versus first-strike monsters of any level", [] {
        Hero hero;
        hero.gainLevel();
        Monster goblin(MonsterType::Goblin, 1);
        hero.addStatus(HeroStatus::FirstStrike);
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
      });
    });
    describe("Heavy Fireball", [] {
      Hero hero(HeroClass::Wizard, HeroRace::Elf);
      hero.addStatus(HeroStatus::HeavyFireball);
      Monster monster(MonsterType::MeatMan, 3);
      it("should increase fireball damage by 4 per caster level", [&] {
        int hp = monster.getHitPoints();
        AssertThat(Cast::targeted(hero, monster, Spell::Burndayraz), Equals(Summary::Safe));
        hp -= 8;
        AssertThat(monster.getHitPoints(), Equals(hp));
        AssertThat(monster.getBurnStackSize(), Equals(2));
        hero.gainLevel();
        hero.gainLevel();
        AssertThat(Cast::targeted(hero, monster, Spell::Burndayraz), Equals(Summary::Safe));
        hp -= 8 * 3 + 2;
        AssertThat(monster.getHitPoints(), Equals(hp));
        AssertThat(monster.getBurnStackSize(), Equals(6));
      });
      it("should let monster retaliate", [&] {
        hero.gainLevel();
        monster.recover(15);
        const int retaliateDamage = monster.getDamage() / 2;
        AssertThat(Cast::targeted(hero, monster, Spell::Burndayraz), Equals(Summary::Safe));
        AssertThat(hero.getHitPointsMax() - hero.getHitPoints(), Equals(retaliateDamage));
        AssertThat(Cast::targeted(hero, monster, Spell::Burndayraz), Equals(Summary::Win));
        AssertThat(hero.getHitPointsMax() - hero.getHitPoints(), Equals(retaliateDamage));
      });
    });
    describe("Knockback", [] { /* TODO */ });
    describe("Life Steal", [] {
      Hero hero;
      hero.addStatus(HeroStatus::LifeSteal);
      Monster cow("", {2, 1000, 1, 0}, {}, {});
      it("should heal hero by 1 per hero level and life steal level and allow overheal", [&] {
        Combat::attack(hero, cow);
        AssertThat(hero.getHitPoints(), Equals(10 + 1 - 1));
        hero.addStatus(HeroStatus::LifeSteal);
        Combat::attack(hero, cow);
        AssertThat(hero.getHitPoints(), Equals(10 + 2 - 1));
        hero.gainLevel();
        Combat::attack(hero, cow);
        AssertThat(hero.getHitPoints(), Equals(20 + 2 * 2 - 1));
      });
      it("should be twice as effective against lower level monsters", [&] {
        hero.gainLevel();
        Combat::attack(hero, cow);
        const int steal = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        AssertThat(hero.getHitPoints(), Equals(30 + steal - 1));
      });
      it("should be applied twice for reflexes", [&] {
        hero.loseHitPointsOutsideOfFight(30);
        const int healthInitial = hero.getHitPoints();
        hero.addStatus(HeroStatus::Reflexes);
        const int steal = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        AssertThat(Combat::attack(hero, cow), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(healthInitial + 2 * steal - 1));
      });
      it("should not heal above 150% max. HP", [&] {
        hero.gainLevel();
        hero.addStatus(HeroStatus::LifeSteal);
        hero.changeDamageBonusPercent(30);
        const int steal = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        AssertThat(steal, IsGreaterThan(hero.getHitPointsMax() / 2 + 1));
        hero.addStatus(HeroStatus::SlowStrike);
        AssertThat(Combat::attack(hero, cow), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() * 3 / 2));
      });
      it("should be applied immediately after hero's attack", [] {
        Hero hero;
        Monster goblin(MonsterType::Goblin, 1);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.loseHitPointsOutsideOfFight(10 - goblin.getDamage());
        goblin.slow();
        AssertThat(Combat::attack(hero, goblin), Equals(Summary::Safe));
        hero.recover(10);
        goblin.recover(10);
        hero.loseHitPointsOutsideOfFight(10 - goblin.getDamage());
        AssertThat(Combat::attack(hero, goblin), Equals(Summary::Death));
      });
      it("should never exceed actual damage done", [] {
        Hero hero;
        hero.gainLevel();
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        const int expected = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        const int limit = hero.getDamageVersusStandard();
        hero.loseHitPointsOutsideOfFight(limit);
        Monster monster("", {1, limit + 1, 0, 0}, {}, {});
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax()));
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Win));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() + 1));
      });
      it("should not work on bloodless monsters", [] {
        Hero hero;
        hero.addStatus(HeroStatus::LifeSteal);
        Monster monster("Bloodless", {1, 10, 3, 0}, {}, std::move(MonsterTraitsBuilder().addBloodless()));
        Combat::attack(hero, monster);
        AssertThat(hero.getHitPoints(), Equals(7));
      });
    });
    describe("Magical Attack", [] {
      it("should convert hero's damage into magical damage", [] {
        Hero hero;
        Monster monster(MonsterType::Wraith, 2);
        AssertThat(monster.getHitPoints(), Equals(11));
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
        AssertThat(monster.getHitPoints(), Equals(7));
        hero.fullHealthAndMana();
        hero.addStatus(HeroStatus::MagicalAttack);
        Combat::attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(2));
      });
    });
    describe("Mana Burned", [] {
      it("should prevent mana recovery by uncovering tiles", [] {
        Hero hero;
        hero.addStatus(HeroStatus::ManaBurned);
        AssertThat(hero.getManaPoints(), Equals(0));
        Combat::uncoverTiles(hero, nullptr, 10);
        AssertThat(hero.getManaPoints(), Equals(0));
      });
      it("should allow other means of mana recovery", [] {
        Hero hero(HeroClass::Thief, HeroRace::Human);
        hero.addStatus(HeroStatus::ManaBurned);
        AssertThat(hero.getManaPoints(), Equals(0));
        hero.addStatus(HeroStatus::Schadenfreude);
        Monster monster(MonsterType::MeatMan, 1);
        Combat::attack(hero, monster);
        AssertThat(hero.getManaPoints(), Equals(monster.getDamage()));
        hero.use(Item::HealthPotion);
        AssertThat(hero.getManaPoints(), Equals(monster.getDamage() + 2));
      });
    });
    describe("Mana Burn Immune", [] {
      it("should prevent being mana burned", [] {
        Hero hero;
        hero.addStatus(HeroStatus::ManaBurnImmune);
        hero.addStatus(HeroStatus::ManaBurned);
        AssertThat(hero.hasStatus(HeroStatus::ManaBurned), Equals(false));
        Hero hero2;
        hero2.addStatus(HeroStatus::ManaBurned);
        AssertThat(hero2.hasStatus(HeroStatus::ManaBurned), Equals(true));
        hero2.addStatus(HeroStatus::ManaBurnImmune);
        AssertThat(hero2.hasStatus(HeroStatus::ManaBurned), Equals(false));
      });
    });
    describe("Might", [] {
      it("should add layer of stone skin when a wall is destroyed", [] {
        Hero hero;
        hero.addStatus(HeroStatus::Might);
        Cast::untargeted(hero, Spell::Endiswal);
        AssertThat(hero.hasStatus(HeroStatus::StoneSkin), IsTrue());
        AssertThat(hero.hasStatus(HeroStatus::Might), IsTrue());
      });
    });
    describe("Pierce Physical", [] { /* TODO */ });
    describe("Poisoned", [] {
      it("should prevent health recovery", [] {
        Hero hero;
        hero.loseHitPointsOutsideOfFight(9);
        Combat::uncoverTiles(hero, nullptr, 1);
        AssertThat(hero.getHitPoints(), Equals(2));
        hero.addStatus(HeroStatus::Poisoned);
        Combat::uncoverTiles(hero, nullptr, 10);
        AssertThat(hero.getHitPoints(), Equals(2));
      });
    });
    describe("Poisonous", [] {
      it("should poison the monster (1 point per hero level)", [] {
        Hero hero({100, 0, 1}, {}, Experience{5});
        hero.addStatus(HeroStatus::Poisonous, 3);
        Monster monster(MonsterType::GooBlob, 4);
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
        AssertThat(monster.getPoisonAmount(), Equals(15));
      });
    });
    describe("Poison Immune", [] {
      it("should prevent being poisoned", [] {
        Hero hero;
        hero.addStatus(HeroStatus::PoisonImmune);
        hero.addStatus(HeroStatus::Poisoned);
        AssertThat(hero.hasStatus(HeroStatus::Poisoned), Equals(false));
        Hero hero2;
        hero2.addStatus(HeroStatus::Poisoned);
        AssertThat(hero2.hasStatus(HeroStatus::Poisoned), Equals(true));
        hero2.addStatus(HeroStatus::PoisonImmune);
        AssertThat(hero2.hasStatus(HeroStatus::Poisoned), Equals(false));
      });
    });
    describe("Reflexes", [] {
      it("should cause 2 hits", [] {
        Hero hero;
        hero.addStatus(HeroStatus::Reflexes);
        Monster monster(1, 2 * hero.getDamageVersusStandard(), 1);
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Win));
      });
    });
    describe("Sanguine", [] { /* TODO */ });
    describe("Schadenfreude", [] {
      it("should refill mana equal to damage received", [] {
        Hero hero;
        hero.loseManaPoints(10);
        hero.addStatus(HeroStatus::Schadenfreude);
        Monster monster("", {1, 6, 8, 0}, {}, {});
        Combat::attack(hero, monster);
        AssertThat(hero.getManaPoints(), Equals(8));
        AssertThat(hero.getHitPoints(), Equals(2));
        AssertThat(hero.hasStatus(HeroStatus::Schadenfreude), IsFalse());
      });
    });
    describe("Slow Strike", [] {
      Hero hero({}, {}, Experience(10));
      Monster meatMan(MonsterType::MeatMan, 1);
      Monster goblin(MonsterType::Goblin, 1);
      it("should give initiative to regular monsters of any level", [&] {
        hero.addStatus(HeroStatus::SlowStrike);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsFalse());
      });
      it("should cancel First Strike", [&] {
        hero.addStatus(HeroStatus::FirstStrike);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsTrue());
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
        hero.removeStatus(HeroStatus::FirstStrike, true);
      });
      it("should give initiative to slowed monsters of any level (regardless of monster's first strike)", [&] {
        meatMan.slow();
        goblin.slow();
        AssertThat(hero.hasInitiativeVersus(meatMan), IsFalse());
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
      });
      it("should not give initiative to slowed monsters when cancelled by First Strike", [&] {
        hero.addStatus(HeroStatus::FirstStrike);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsTrue());
        AssertThat(hero.hasInitiativeVersus(goblin), IsTrue());
      });
    });
    describe("Spirit Strength", [] {
      Hero hero;
      it("should add damage points to the next attack", [&] {
        AssertThat(hero.getBaseDamage(), Equals(5));
        hero.addStatus(HeroStatus::SpiritStrength, 7);
        AssertThat(hero.getBaseDamage(), Equals(12));
        hero.changeDamageBonusPercent(100);
        AssertThat(hero.getDamageVersusStandard(), Equals(24));
      });
      it("should wear off", [&] {
        Monster monster("", {2, 35, 1, 0}, {}, {});
        hero.addStatus(HeroStatus::Reflexes);
        AssertThat(hero.hasStatus(HeroStatus::SpiritStrength), IsTrue());
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
        AssertThat(hero.hasStatus(HeroStatus::SpiritStrength), IsFalse());
      });
    });
    describe("Stone Skin", [] {
      Hero hero;
      hero.gainLevel();
      it("should add 20% physical resistance per stack", [&] {
        hero.addStatus(HeroStatus::StoneSkin);
        AssertThat(hero.getPhysicalResistPercent(), Equals(20));
        hero.addStatus(HeroStatus::StoneSkin, 2);
        AssertThat(hero.getPhysicalResistPercent(), Equals(60));
        hero.addStatus(HeroStatus::StoneSkin);
        AssertThat(hero.getPhysicalResistPercent(), Equals(65));
      });
      it("should wear off when hit", [&] {
        Monster monster(MonsterType::MeatMan, 1);
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(19));
        AssertThat(hero.hasStatus(HeroStatus::StoneSkin), IsFalse());
        hero.addStatus(HeroStatus::StoneSkin);
        AssertThat(Combat::attack(hero, monster), Equals(Summary::Win));
        AssertThat(hero.hasStatus(HeroStatus::StoneSkin), IsTrue());
      });
    });
    describe("Weakened", [] {
      it("should reduce base damage by one per stack level", [] {
        Hero hero;
        hero.changeDamageBonusPercent(100);
        AssertThat(hero.getDamageBonusPercent(), Equals(100));
        const int base_initial = hero.getBaseDamage();
        const int damage_initial = hero.getDamageVersusStandard();
        hero.addStatus(HeroStatus::Weakened);
        AssertThat(hero.getBaseDamage(), Equals(base_initial - 1));
        AssertThat(hero.getDamageVersusStandard(), Equals(damage_initial - 2));
        AssertThat(hero.getDamageBonusPercent(), Equals(100));
        hero.addStatus(HeroStatus::Weakened);
        AssertThat(hero.getBaseDamage(), Equals(base_initial - 2));
      });
      it("should not reduce damage below zero", [] {
        Hero hero;
        hero.addStatus(HeroStatus::Weakened, hero.getBaseDamage() + 1);
        AssertThat(hero.getBaseDamage(), Is().Not().LessThan(0));
        AssertThat(hero.getDamageVersusStandard(), Is().Not().LessThan(0));
      });
    });
  });
}

void testPotions()
{
  describe("Health Potion", [] {
    it("restores 40% of max HP", [] {
      Hero hero;
      hero.setHitPointsMax(99);
      const int expected = hero.getHitPoints() + 39;
      hero.use(Item::HealthPotion);
      AssertThat(hero.getHitPoints(), Equals(expected));
    });
    it("removes poison", [] {
      Hero hero;
      hero.addStatus(HeroStatus::Poisoned);
      hero.use(Item::HealthPotion);
      AssertThat(hero.hasStatus(HeroStatus::Poisoned), IsFalse());
    });
    it("does not overheal", [] {
      Hero hero;
      hero.healHitPoints(1, true);
      AssertThat(hero.getHitPoints(), Equals(11));
      hero.use(Item::HealthPotion);
      AssertThat(hero.getHitPoints(), Equals(11));
    });
  });
  describe("Mana Potion", [] {
    it("restores 40% of max MP", [] {
      Hero hero;
      hero.setManaPointsMax(99);
      const int expected = hero.getManaPoints() + 39;
      hero.use(Item::ManaPotion);
      AssertThat(hero.getManaPoints(), Equals(expected));
      hero.use(Item::ManaPotion);
      hero.use(Item::ManaPotion);
      AssertThat(hero.getManaPoints(), Equals(99));
    });
    it("removes mana burn", [] {
      Hero hero;
      hero.addStatus(HeroStatus::ManaBurned);
      hero.use(Item::ManaPotion);
      AssertThat(hero.hasStatus(HeroStatus::ManaBurned), IsFalse());
    });
  });
  describe("Fortitude Tonic", [] {
    it("removes poison and weakening", [] {
      Hero hero;
      hero.addStatus(HeroStatus::Poisoned);
      hero.addStatus(HeroStatus::Weakened, 50);
      hero.use(Item::FortitudeTonic);
      AssertThat(hero.hasStatus(HeroStatus::Poisoned), IsFalse());
      AssertThat(hero.hasStatus(HeroStatus::Weakened), IsFalse());
    });
  });
  describe("Burn Salve", [] {
    it("removes mana burn and corrosion", [] {
      Hero hero;
      hero.addStatus(HeroStatus::ManaBurned);
      hero.addStatus(HeroStatus::Corrosion, 50);
      hero.use(Item::BurnSalve);
      AssertThat(hero.hasStatus(HeroStatus::ManaBurned), IsFalse());
      AssertThat(hero.hasStatus(HeroStatus::Corrosion), IsFalse());
    });
  });
  describe("Strength Potion", [] {
    it("Converts all mana into spirit strength", [] {
      Hero hero;
      hero.gainLevel();
      hero.setManaPointsMax(123);
      hero.fullHealthAndMana();
      hero.use(Item::StrengthPotion);
      AssertThat(hero.getManaPoints(), Equals(0));
      AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(123 + hero.getLevel()));
      AssertThat(hero.getDamageVersusStandard(), Equals(135));
    });
  });
  describe("Schadenfreude", [] {
    it("adds Schadenfreude status", [] {
      Hero hero;
      hero.use(Item::Schadenfreude);
      AssertThat(hero.hasStatus(HeroStatus::Schadenfreude), IsTrue());
    });
  });
  describe("Quicksilver Potion", [] {
    it("adds 50% dodge change (temporary)", [] {
      Hero hero;
      hero.use(Item::QuicksilverPotion);
      AssertThat(hero.getDodgeChancePercent(), Equals(50));
      AssertThat(hero.hasStatus(HeroStatus::DodgePermanent), Equals(0));
    });
    it("adds dodge prediction", [] {
      Hero hero;
      hero.use(Item::QuicksilverPotion);
      AssertThat(hero.hasStatus(HeroStatus::DodgePrediction), IsTrue());
    });
  });
  describe("Reflex Potion", [] {
    it("adds Reflexes status", [] {
      Hero hero;
      hero.use(Item::ReflexPotion);
      AssertThat(hero.hasStatus(HeroStatus::Reflexes), IsTrue());
    });
  });
  describe("Can Of Whupaz", [] {
    it("adds Crushing Blow status", [] {
      Hero hero;
      hero.use(Item::CanOfWhupaz);
      AssertThat(hero.hasStatus(HeroStatus::CrushingBlow), IsTrue());
    });
  });
}

void testCombatInitiative()
{
  describe("Initiative", [] {
    Hero hero;
    Monster boss(10, 10, 3);
    Monster monster(1, 10, 3);
    it("should go to the monster of higher level", [&] { //
      AssertThat(hero.hasInitiativeVersus(boss), IsFalse());
    });
    it("should go to the monster of equal level", [&] { //
      AssertThat(hero.hasInitiativeVersus(monster), IsFalse());
    });
    it("should go to the hero if he has higher level", [&] {
      hero.gainLevel();
      AssertThat(hero.hasInitiativeVersus(monster), IsTrue());
    });
  });
}

void testDungeonBasics()
{
  describe("Dungeon", [] {
    Dungeon dungeon(3, 3);
    auto monster = std::make_shared<Monster>(3, 30, 10);
    auto monster2 = std::make_shared<Monster>(*monster);
    it("should allow adding monsters if there is space", [&] {
      dungeon.add(monster, dungeon.randomFreePosition().value());
      AssertThat(dungeon.getMonsters().size(), Equals(1));
      for (unsigned n = 2; n <= 9; ++n)
      {
        AssertThat(dungeon.randomFreePosition().has_value(), IsTrue());
        dungeon.add(monster2, dungeon.randomFreePosition().value());
        AssertThat(dungeon.getMonsters().size(), Equals(n));
      }
      AssertThat(dungeon.randomFreePosition().has_value(), IsFalse());
      AssertThat(dungeon.isFree({0, 0}), IsFalse());
    });
    it("should not be revealed initially", [&] { AssertThat(dungeon.isRevealed({1, 1}), IsFalse()); });
    it("should consider a square with a defeated monster as free", [&] {
      monster->takeDamage(100, false);
      AssertThat(monster->isDefeated(), IsTrue());
      dungeon.update();
      AssertThat(dungeon.randomFreePosition().has_value(), IsTrue());
    });
    auto hero = std::make_shared<Hero>();
    it("should consider the hero's square as occupied", [&] {
      dungeon.setHero(hero, dungeon.randomFreePosition().value());
      AssertThat(dungeon.randomFreePosition().has_value(), IsFalse());
    });
    it("should be revealed around the hero's position", [&] { AssertThat(dungeon.isRevealed({1, 1}), IsTrue()); });
  });

  describe("Pathfinding", [] {
    Dungeon dungeon(10, 10);
    auto hero = std::make_shared<Hero>();
    dungeon.setHero(hero, Position(0, 0));
    it("should find paths if there are no obstacles", [&] { AssertThat(dungeon.isConnected({9, 9}), IsTrue()); });
    it("should consider squares inaccessible if there are no revealed paths to them", [&] {
      AssertThat(dungeon.isAccessible({9, 9}), IsFalse());
    });
    it("should consider squares accessible if there is a revealed path", [&] {
      for (int x = 2; x < 10; ++x)
        dungeon.reveal({x, x});
      AssertThat(dungeon.isAccessible({9, 9}), IsTrue());
    });
  });
}

go_bandit([] {
  testHeroExperience();
  testMonsterBasics();
  testDefenceBasics();
  testMelee();
  testStatusEffects();
  testPotions();
  testCombatInitiative();
  testDungeonBasics();
});

int main(int argc, char* argv[])
{
  // Run the tests
  return bandit::run(argc, argv);
}
