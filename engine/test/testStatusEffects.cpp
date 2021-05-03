#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"
#include "engine/Magic.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
  SimpleResources resources{{}};
} // namespace

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
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.isDefeated(), IsTrue());
        AssertThat(hero.doesMagicalDamage(), IsFalse());
        AssertThat(hero.hasStatus(HeroStatus::ConsecratedStrike), IsFalse());
      });
    });
    describe("Corrosive Strike", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      hero.addStatus(HeroStatus::CorrosiveStrike);
      it("should corrode monster", [&] {
        Monster monster(MonsterType::MeatMan, 2);
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.getCorroded(), Equals(1));
        hero.addStatus(HeroStatus::FirstStrikeTemporary);
        hero.addStatus(HeroStatus::CorrosiveStrike, 2);
        hero.refillHealthAndMana();
        Combat::attack(hero, monster, noOtherMonsters);
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
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.getHitPoints(), Equals(monster.getHitPointsMax() * 3 / 4));
      });
      it("should ignore physical immunity", [&] {
        Monster monster("", MonsterStats{1, 100, 1, 0}, Defence{100, 100}, {});
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.getHitPoints(), Equals(75));
      });
      it("should ignore magical immunity", [&] {
        Monster monster("", MonsterStats{1, 100, 1, 0}, Defence{100, 100}, {});
        hero.addStatus(HeroStatus::MagicalAttack);
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.getHitPoints(), Equals(75));
      });
      it("should not affect monsters below 75% health", [&] {
        Monster monster(MonsterType::MeatMan, 1);
        monster.takeDamage(monster.getHitPoints() - 1, DamageType::Physical);
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.getHitPoints(), Equals(1));
      });
      it("should wear off", [&] {
        Monster monster(MonsterType::MeatMan, 10);
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(hero.hasStatus(HeroStatus::CrushingBlow), IsFalse());
      });
    });
    describe("Cursed", [] {
      it("should negate resistances", [] {
        Hero hero;
        hero.setPhysicalResistPercent(50);
        hero.takeDamage(4, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10 - 4 / 2));
        hero.addStatus(HeroDebuff::Cursed, noOtherMonsters);
        hero.takeDamage(4, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10 - 4 / 2 - 4));
      });
      it("should be added/removed when cursed/not-cursed monster is defeated", [] {
        Hero hero;
        hero.changeBaseDamage(100);
        Monster monster("", MonsterStats{1, 10, 3, 0}, {}, MonsterTraitsBuilder().addCurse());
        Combat::attack(hero, monster, noOtherMonsters);
        // One curse from hit, one from killing
        AssertThat(hero.getStatusIntensity(HeroDebuff::Cursed), Equals(2));
        Monster monster2(1, 10, 3);
        Combat::attack(hero, monster2, noOtherMonsters);
        AssertThat(hero.getStatusIntensity(HeroDebuff::Cursed), Equals(1));
      });
      it("should take effect immediately after hero's attack", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        const int health = hero.getHitPoints();
        hero.setPhysicalResistPercent(50);
        Monster monster("", MonsterStats(1, 100, 10, 0), {}, MonsterTraitsBuilder().addCurse());
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(hero.hasStatus(HeroDebuff::Cursed), IsTrue());
        AssertThat(hero.getHitPoints(), Equals(health - 10));
      });
    });
    describe("Curse Immune", [] {
      it("should prevent being cursed", [] {
        Hero hero;
        hero.addStatus(HeroStatus::CurseImmune);
        hero.addStatus(HeroDebuff::Cursed, noOtherMonsters);
        AssertThat(hero.hasStatus(HeroDebuff::Cursed), Equals(false));
        Hero hero2;
        hero2.addStatus(HeroDebuff::Cursed, noOtherMonsters);
        AssertThat(hero2.hasStatus(HeroDebuff::Cursed), Equals(true));
        hero2.addStatus(HeroStatus::CurseImmune);
        AssertThat(hero2.hasStatus(HeroDebuff::Cursed), Equals(false));
      });
    });
    describe("Death Gaze", [] { /* TODO */ });
    describe("Death Gaze Immune", [] {
      it("should prevent petrification", [] {
        Hero hero;
        hero.takeDamage(6, DamageType::Physical, noOtherMonsters);
        hero.addStatus(HeroStatus::DeathGazeImmune);
        Monster gorgon(MonsterType::Gorgon, 1);
        AssertThat(Combat::attack(hero, gorgon, noOtherMonsters), Equals(Summary::Win));
      });
    });
    describe("Dodge", [] {
      it("should result in hero not taking hit", [] {
        Hero hero;
        hero.addStatus(HeroStatus::DodgeTemporary, 100);
        Monster boss(MonsterType::Gorgon, 10);
        AssertThat(hero.getDodgeChancePercent(), Equals(100));
        AssertThat(Combat::attack(hero, boss, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getDodgeChancePercent(), Equals(0));
        hero.addStatus(HeroStatus::DodgePermanent, 100);
        AssertThat(hero.getDodgeChancePercent(), Equals(100));
        AssertThat(Combat::attack(hero, boss, noOtherMonsters), Equals(Summary::Safe));
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
            // trigger reroll
            hero.addStatus(HeroStatus::DodgeTemporary);
            hero.reduceStatus(HeroStatus::DodgeTemporary);
          }
          const int tolerance = 2 * (11 - chance / 10);
          const int expectedWithTolerance = 100 / chance * tolerance;
          AssertThat(attempts, IsLessThanOrEqualTo(expectedWithTolerance));
        }
      });
      it("chance should be rerolled after each attack", [] {
        Hero hero;
        hero.addDodgeChancePercent(50, true);
        hero.addStatus(HeroStatus::DodgePrediction);
        hero.gainLevel(noOtherMonsters);
        bool dodgeNext = hero.predictDodgeNext();
        int changes = 0;
        for (int i = 0; i < 1000; ++i)
        {
          // Alternate between level 1 and 2 monsters to change attack order
          Monster monster(MonsterType::MeatMan, 1 + i % 2);
          Combat::attack(hero, monster, noOtherMonsters);
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
            hero.refillHealthAndMana();
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
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(monster.getHitPointsMax() - monster.getHitPoints(), Equals(2 * hero.getDamageVersusStandard()));
      });
    });
    describe("Exhausted", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      hero.gainLevel(noOtherMonsters);
      hero.addTrait(HeroTrait::Damned);
      it("should be computed automatically for damned hero", [&] {
        AssertThat(hero.hasStatus(HeroStatus::Exhausted), IsFalse());
        hero.loseHitPointsOutsideOfFight(3, noOtherMonsters);
        AssertThat(hero.hasStatus(HeroStatus::Exhausted), IsTrue());
      });
      it("should limit health recovery", [&] {
        AssertThat(hero.getHitPoints(), Equals(27));
        hero.recover(1);
        AssertThat(hero.getHitPoints(), Equals(28));
      });
      it("should prevent mana recovery", [&] {
        hero.loseManaPoints(1);
        AssertThat(hero.getManaPoints(), Equals(9));
        hero.recover(2);
        AssertThat(hero.getManaPoints(), Equals(9));
        AssertThat(hero.hasStatus(HeroStatus::Exhausted), IsFalse());
        hero.recover(1);
        AssertThat(hero.getManaPoints(), Equals(10));
      });
    });
    describe("First Strike", [] {
      it("should give initiative versus regular monsters of any level", [&] {
        Hero hero;
        Monster boss(MonsterType::Goat, 10);
        hero.addStatus(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(boss), IsTrue());
      });
      it("should not give initiative versus first-strike monsters of any level", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        Monster goblin(MonsterType::Goblin, 1);
        hero.addStatus(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
      });
    });
    describe("Heavy Fireball", [] {
      Hero hero(HeroClass::Wizard, HeroRace::Elf);
      hero.addStatus(HeroStatus::HeavyFireball);
      Monster monster(MonsterType::MeatMan, 3);
      it("should increase fireball damage by 4 per caster level", [&] {
        int hp = monster.getHitPoints();
        AssertThat(Magic::cast(hero, monster, Spell::Burndayraz, noOtherMonsters, resources), Equals(Summary::Safe));
        hp -= 8;
        AssertThat(monster.getHitPoints(), Equals(hp));
        AssertThat(monster.getBurnStackSize(), Equals(2));
        hero.gainLevel(noOtherMonsters);
        hero.gainLevel(noOtherMonsters);
        AssertThat(Magic::cast(hero, monster, Spell::Burndayraz, noOtherMonsters, resources), Equals(Summary::Safe));
        hp -= 8 * 3 + 2;
        AssertThat(monster.getHitPoints(), Equals(hp));
        AssertThat(monster.getBurnStackSize(), Equals(6));
      });
      it("should let monster retaliate", [&] {
        hero.gainLevel(noOtherMonsters);
        monster.recover(15);
        const int retaliateDamage = monster.getDamage() / 2;
        AssertThat(Magic::cast(hero, monster, Spell::Burndayraz, noOtherMonsters, resources), Equals(Summary::Safe));
        AssertThat(hero.getHitPointsMax() - hero.getHitPoints(), Equals(retaliateDamage));
        AssertThat(Magic::cast(hero, monster, Spell::Burndayraz, noOtherMonsters, resources), Equals(Summary::Win));
        AssertThat(hero.getHitPointsMax() - hero.getHitPoints(), Equals(retaliateDamage));
      });
    });
    describe("Knockback", [] { /* TODO */ });
    describe("Life Steal", [] {
      Hero hero;
      hero.addStatus(HeroStatus::LifeSteal);
      Monster cow("", {2, 1000, 1, 0}, {}, {});
      it("should heal hero by 1 per hero level and life steal level and allow overheal", [&] {
        Combat::attack(hero, cow, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10 + 1 - 1));
        hero.addStatus(HeroStatus::LifeSteal);
        Combat::attack(hero, cow, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10 + 2 - 1));
        hero.gainLevel(noOtherMonsters);
        Combat::attack(hero, cow, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(20 + 2 * 2 - 1));
      });
      it("should be twice as effective against lower level monsters", [&] {
        hero.gainLevel(noOtherMonsters);
        Combat::attack(hero, cow, noOtherMonsters);
        const int steal = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        AssertThat(hero.getHitPoints(), Equals(30 + steal - 1));
      });
      it("should be applied twice for reflexes", [&] {
        hero.loseHitPointsOutsideOfFight(30, noOtherMonsters);
        const int healthInitial = hero.getHitPoints();
        hero.addStatus(HeroStatus::Reflexes);
        const int steal = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        AssertThat(Combat::attack(hero, cow, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(healthInitial + 2 * steal - 1));
      });
      it("should not heal above 150% max. HP", [&] {
        hero.gainLevel(noOtherMonsters);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.changeDamageBonusPercent(30);
        const int steal = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        AssertThat(steal, IsGreaterThan(hero.getHitPointsMax() / 2 + 1));
        hero.addStatus(HeroStatus::SlowStrike);
        AssertThat(Combat::attack(hero, cow, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() * 3 / 2));
      });
      it("should be applied immediately after hero's attack", [] {
        Hero hero;
        Monster goblin(MonsterType::Goblin, 1);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.loseHitPointsOutsideOfFight(10 - goblin.getDamage(), noOtherMonsters);
        goblin.slow();
        AssertThat(Combat::attack(hero, goblin, noOtherMonsters), Equals(Summary::Safe));
        hero.recover(10);
        goblin.recover(10);
        hero.loseHitPointsOutsideOfFight(10 - goblin.getDamage(), noOtherMonsters);
        AssertThat(Combat::attack(hero, goblin, noOtherMonsters), Equals(Summary::Death));
      });
      it("should never exceed actual damage done", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        hero.addStatus(HeroStatus::LifeSteal);
        const int expected = 2 * hero.getLevel() * hero.getStatusIntensity(HeroStatus::LifeSteal);
        const int limit = hero.getDamageVersusStandard();
        AssertThat(expected > limit, IsTrue());
        hero.loseHitPointsOutsideOfFight(limit, noOtherMonsters);
        Monster monster("", {1, limit + 1, 0, 0}, {}, {});
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax()));
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Win));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() + 1));
      });
      it("should not work on bloodless monsters", [] {
        Hero hero;
        hero.addStatus(HeroStatus::LifeSteal);
        Monster monster("Bloodless", {1, 10, 3, 0}, {}, MonsterTraitsBuilder().addBloodless());
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(7));
      });
    });
    describe("Magical Attack", [] {
      it("should convert hero's damage into magical damage", [] {
        Hero hero;
        Monster monster(MonsterType::Wraith, 2);
        AssertThat(monster.getHitPoints(), Equals(11));
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(monster.getHitPoints(), Equals(7));
        hero.refillHealthAndMana();
        hero.addStatus(HeroStatus::MagicalAttack);
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.getHitPoints(), Equals(2));
      });
    });
    describe("Mana Burned", [] {
      it("should prevent mana recovery by uncovering tiles", [] {
        Hero hero;
        hero.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(0));
        hero.recover(10);
        AssertThat(hero.getManaPoints(), Equals(0));
      });
      it("should allow other means of mana recovery", [] {
        Hero hero(HeroClass::Thief, HeroRace::Human);
        hero.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(0));
        hero.addStatus(HeroStatus::Schadenfreude);
        Monster monster(MonsterType::MeatMan, 1);
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(monster.getDamage()));
        hero.use(Potion::HealthPotion, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(monster.getDamage() + 2));
      });
    });
    describe("Mana Burn Immune", [] {
      it("should prevent being mana burned", [] {
        Hero hero;
        hero.addStatus(HeroStatus::ManaBurnImmune);
        hero.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero.hasStatus(HeroDebuff::ManaBurned), Equals(false));
        Hero hero2;
        hero2.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero2.hasStatus(HeroDebuff::ManaBurned), Equals(true));
        hero2.addStatus(HeroStatus::ManaBurnImmune);
        AssertThat(hero2.hasStatus(HeroDebuff::ManaBurned), Equals(false));
      });
    });
    describe("Might", [] {
      Hero hero;
      it("should increase damage of next attack by 30%", [&] {
        hero.addStatus(HeroStatus::Might);
        AssertThat(hero.getDamageBonusPercent(), Equals(30));
      });
      it("should lower enemies resistances by 3%", [&] {
        Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{50, 75}, {});
        Combat::attack(hero, monster, noOtherMonsters);
        AssertThat(monster.getPhysicalResistPercent(), Equals(47));
        AssertThat(monster.getMagicalResistPercent(), Equals(72));
      });
      it("should wear off", [&] { AssertThat(hero.hasStatus(HeroStatus::Might), IsFalse()); });
    });
    describe("Pierce Physical", [] { /* TODO */ });
    describe("Poisoned", [] {
      it("should prevent health recovery", [] {
        Hero hero;
        hero.loseHitPointsOutsideOfFight(9, noOtherMonsters);
        hero.recover(1);
        AssertThat(hero.getHitPoints(), Equals(2));
        hero.addStatus(HeroDebuff::Poisoned, noOtherMonsters);
        hero.recover(10);
        AssertThat(hero.getHitPoints(), Equals(2));
      });
    });
    describe("Poisonous", [] {
      it("should poison the monster (1 point per hero level)", [] {
        Hero hero({100, 0, 1}, {}, Experience{5});
        hero.addStatus(HeroStatus::Poisonous, 3);
        Monster monster(MonsterType::GooBlob, 4);
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(monster.getPoisonAmount(), Equals(15));
      });
    });
    describe("Poison Immune", [] {
      it("should prevent being poisoned", [] {
        Hero hero;
        hero.addStatus(HeroStatus::PoisonImmune);
        hero.addStatus(HeroDebuff::Poisoned, noOtherMonsters);
        AssertThat(hero.hasStatus(HeroDebuff::Poisoned), Equals(false));
        Hero hero2;
        hero2.addStatus(HeroDebuff::Poisoned, noOtherMonsters);
        AssertThat(hero2.hasStatus(HeroDebuff::Poisoned), Equals(true));
        hero2.addStatus(HeroStatus::PoisonImmune);
        AssertThat(hero2.hasStatus(HeroDebuff::Poisoned), Equals(false));
      });
    });
    describe("Reflexes", [] {
      it("should cause 2 hits", [] {
        Hero hero;
        hero.addStatus(HeroStatus::Reflexes);
        Monster monster(1, 2 * hero.getDamageVersusStandard(), 1);
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Win));
      });
    });
    describe("Sanguine", [] { /* TODO */ });
    describe("Schadenfreude", [] {
      it("should refill mana equal to damage received", [] {
        Hero hero;
        hero.loseManaPoints(10);
        hero.addStatus(HeroStatus::Schadenfreude);
        Monster monster("", {1, 6, 8, 0}, {}, {});
        Combat::attack(hero, monster, noOtherMonsters);
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
        hero.addStatus(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsTrue());
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
        hero.resetStatus(HeroStatus::FirstStrikeTemporary);
      });
      it("should give initiative to slowed monsters of any level (regardless of monster's first strike)", [&] {
        meatMan.slow();
        goblin.slow();
        AssertThat(hero.hasInitiativeVersus(meatMan), IsFalse());
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
      });
      it("should not give initiative to slowed monsters when cancelled by First Strike", [&] {
        hero.addStatus(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsTrue());
        AssertThat(hero.hasInitiativeVersus(goblin), IsTrue());
      });
    });
    describe("Spirit Strength", [] {
      Hero hero;
      it("should add base damage for the next attack", [&] {
        AssertThat(hero.getBaseDamage(), Equals(5));
        hero.addStatus(HeroStatus::SpiritStrength, 7);
        AssertThat(hero.getBaseDamage(), Equals(12));
        hero.changeDamageBonusPercent(100);
        AssertThat(hero.getDamageVersusStandard(), Equals(24));
      });
      it("should wear off after the next physical attack", [&] {
        Monster monster("", {2, 40, 1, 0}, {}, {});
        hero.addStatus(HeroStatus::Reflexes);
        AssertThat(Magic::cast(hero, monster, Spell::Burndayraz, noOtherMonsters, resources), Equals(Summary::Safe));
        AssertThat(hero.hasStatus(HeroStatus::SpiritStrength), IsTrue());
        AssertThat(monster.getHitPoints(), Equals(36));
        AssertThat(monster.getBurnStackSize(), Equals(1));
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
        const int expected = 36 - 1 /* burn stack */ - 24 /* spirit attack */ - 10 /* regular attack */;
        AssertThat(monster.getHitPoints(), Equals(expected));
        AssertThat(hero.hasStatus(HeroStatus::SpiritStrength), IsFalse());
      });
      it("should wear off after casting Pisorf", [] {
        Hero hero(HeroClass::Transmuter, HeroRace::Human);
        Monster monster("", {1, 31, 1, 0}, {}, {});
        hero.addConversionPoints(100, noOtherMonsters);
        AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(11));
        AssertThat(hero.getBaseDamage(), Equals(16));
        hero.changeBaseDamage(34);
        AssertThat(hero.getBaseDamage(), Equals(50));
        hero.use(Potion::ManaPotion, noOtherMonsters);
        resources.numWalls += 1;
        AssertThat(Magic::cast(hero, monster, Spell::Pisorf, noOtherMonsters, resources), Equals(Summary::Safe));
        AssertThat(hero.hasStatus(HeroStatus::SpiritStrength), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1 /* 31 - 60% * 50 */));
      });
      it("should not pile up", [] {
        Hero hero(HeroClass::Transmuter, HeroRace::Human);
        hero.addConversionPoints(100, noOtherMonsters);
        AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(11));
        hero.addConversionPoints(100, noOtherMonsters);
        AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(11));
        hero.recover(10);
        hero.use(Potion::StrengthPotion, noOtherMonsters);
        AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(11));
        hero.use(Potion::StrengthPotion, noOtherMonsters);
        AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(11));
      });
    });
    describe("Stone Skin", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
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
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(19));
        AssertThat(hero.hasStatus(HeroStatus::StoneSkin), IsFalse());
        hero.addStatus(HeroStatus::StoneSkin);
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Win));
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
        hero.addStatus(HeroDebuff::Weakened, noOtherMonsters);
        AssertThat(hero.getBaseDamage(), Equals(base_initial - 1));
        AssertThat(hero.getDamageVersusStandard(), Equals(damage_initial - 2));
        AssertThat(hero.getDamageBonusPercent(), Equals(100));
        hero.addStatus(HeroDebuff::Weakened, noOtherMonsters);
        AssertThat(hero.getBaseDamage(), Equals(base_initial - 2));
      });
      it("should not reduce damage below zero", [] {
        Hero hero;
        hero.addStatus(HeroDebuff::Weakened, noOtherMonsters, hero.getBaseDamage() + 1);
        AssertThat(hero.getBaseDamage(), Is().Not().LessThan(0));
        AssertThat(hero.getDamageVersusStandard(), Is().Not().LessThan(0));
      });
    });
  });
}
