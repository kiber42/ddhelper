#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Magic.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"
#include "engine/Resources.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
  SimpleResources resources;

  auto attack(Hero& hero, Monster& monster) { return Combat::attack(hero, monster, noOtherMonsters, resources); }

  auto cast(Hero& hero, Monster& monster, Spell spell)
  {
    return Magic::cast(hero, monster, spell, noOtherMonsters, resources);
  }
} // namespace

void testStatusEffects()
{
  describe("Status", [] {
    describe("Burning Strike", [] { /* TODO */ });
    describe("Consecrated Strike", [] {
      Hero hero;
      it("should result in magical damage", [&] {
        AssertThat(hero.doesMagicalDamage(), IsFalse());
        hero.add(HeroStatus::ConsecratedStrike);
        AssertThat(hero.doesMagicalDamage(), IsTrue());
      });
      it("should wear off", [&] {
        Monster monster("", {Level{1}, 5_HP, 1_damage}, {100_physicalresist}, {});
        attack(hero, monster);
        AssertThat(monster.isDefeated(), IsTrue());
        AssertThat(hero.doesMagicalDamage(), IsFalse());
        AssertThat(hero.has(HeroStatus::ConsecratedStrike), IsFalse());
      });
    });
    describe("Corrosive Strike", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      hero.add(HeroStatus::CorrosiveStrike);
      it("should corrode monster", [&] {
        Monster monster(MonsterType::MeatMan, Level{2});
        attack(hero, monster);
        AssertThat(monster.getCorroded(), Equals(1u));
        hero.add(HeroStatus::FirstStrikeTemporary);
        hero.add(HeroStatus::CorrosiveStrike, 2);
        hero.refillHealthAndMana();
        attack(hero, monster);
        AssertThat(monster.getCorroded(), Equals(4u));
        AssertThat(monster.getHitPointsMax() - monster.getHitPoints(),
                   Equals(2u * hero.getDamageVersusStandard() + 1u));
      });
    });
    describe("Crushing Blow", [] {
      Hero hero;
      before_each([&] {
        hero.add(HeroStatus::CrushingBlow);
        hero.add(HeroStatus::DeathProtection);
      });
      it("should reduce the monster's health to 75%", [&] {
        Monster monster(MonsterType::MeatMan, Level{10});
        attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(monster.getHitPointsMax() * 3u / 4u));
      });
      it("should ignore physical immunity", [&] {
        Monster monster("", {Level{1}, 100_HP, 1_damage}, {100_physicalresist, 100_magicalresist}, {});
        attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(75u));
      });
      it("should ignore magical immunity", [&] {
        Monster monster("", {Level{1}, 100_HP, 1_damage}, {100_physicalresist, 100_magicalresist}, {});
        hero.add(HeroStatus::MagicalAttack);
        attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(75u));
      });
      it("should not affect monsters below 75% health", [&] {
        Monster monster(MonsterType::MeatMan, Level{1});
        monster.takeDamage(monster.getHitPoints() - 1, DamageType::Physical);
        attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(1u));
      });
      it("should wear off", [&] {
        Monster monster(MonsterType::MeatMan, Level{10});
        attack(hero, monster);
        AssertThat(hero.has(HeroStatus::CrushingBlow), IsFalse());
      });
    });
    describe("Cursed", [] {
      it("should negate resistances", [] {
        Hero hero;
        hero.setPhysicalResistPercent(50);
        hero.takeDamage(4, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10u - 4u / 2u));
        hero.add(HeroDebuff::Cursed, noOtherMonsters);
        hero.takeDamage(4, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10u - 4u / 2u - 4u));
      });
      it("should be added/removed when cursed/not-cursed monster is defeated", [] {
        Hero hero;
        hero.changeBaseDamage(100);
        Monster monster("", {Level{1}, 10_HP, 3_damage}, {}, {MonsterTrait::CurseBearer});
        attack(hero, monster);
        // One curse from hit, one from killing
        AssertThat(hero.getIntensity(HeroDebuff::Cursed), Equals(2u));
        Monster monster2(1, 10, 3);
        attack(hero, monster2);
        AssertThat(hero.getIntensity(HeroDebuff::Cursed), Equals(1u));
      });
      it("should take effect immediately after hero's attack", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        const auto health = hero.getHitPoints();
        hero.setPhysicalResistPercent(50);
        Monster monster("", {Level{1}, 100_HP, 10_damage}, {}, {MonsterTrait::CurseBearer});
        attack(hero, monster);
        AssertThat(hero.has(HeroDebuff::Cursed), IsTrue());
        AssertThat(hero.getHitPoints(), Equals(health - 10u));
      });
    });
    describe("Curse Immune", [] {
      it("should prevent being cursed", [] {
        Hero hero;
        hero.add(HeroStatus::CurseImmune);
        hero.add(HeroDebuff::Cursed, noOtherMonsters);
        AssertThat(hero.has(HeroDebuff::Cursed), Equals(false));
        Hero hero2;
        hero2.add(HeroDebuff::Cursed, noOtherMonsters);
        AssertThat(hero2.has(HeroDebuff::Cursed), Equals(true));
        hero2.add(HeroStatus::CurseImmune);
        AssertThat(hero2.has(HeroDebuff::Cursed), Equals(false));
      });
    });
    describe("Death Gaze", [] { /* TODO */ });
    describe("Death Gaze Immune", [] {
      it("should prevent petrification", [] {
        Hero hero;
        hero.takeDamage(6, DamageType::Physical, noOtherMonsters);
        hero.add(HeroStatus::DeathGazeImmune);
        Monster gorgon(MonsterType::Gorgon, Level{1});
        AssertThat(attack(hero, gorgon), Equals(Summary::Win));
      });
    });
    describe("Dodge", [] {
      it("should result in hero not taking hit", [] {
        Hero hero;
        hero.add(HeroStatus::DodgeTemporary, 100);
        Monster boss(MonsterType::Gorgon, Level{10});
        AssertThat(hero.getDodgeChancePercent(), Equals(100u));
        AssertThat(attack(hero, boss), Equals(Summary::Safe));
        AssertThat(hero.getDodgeChancePercent(), Equals(0u));
        hero.add(HeroStatus::DodgePermanent, 100);
        AssertThat(hero.getDodgeChancePercent(), Equals(100u));
        AssertThat(attack(hero, boss), Equals(Summary::Safe));
        AssertThat(hero.getDodgeChancePercent(), Equals(100u));
      });
      it("should succeed with given probability", [] {
        Hero hero;
        for (int chance = 10; chance <= 100; chance += 10)
        {
          hero.add(HeroStatus::DodgePrediction);
          hero.addDodgeChancePercent(10, false);
          int attempts = 1;
          while (!hero.predictDodgeNext() && attempts < 100)
          {
            ++attempts;
            // trigger reroll
            hero.add(HeroStatus::DodgeTemporary);
            hero.reduce(HeroStatus::DodgeTemporary);
          }
          const int tolerance = 2 * (11 - chance / 10);
          const int expectedWithTolerance = 100 / chance * tolerance;
          AssertThat(attempts, IsLessThanOrEqualTo(expectedWithTolerance));
        }
      });
      it("chance should be rerolled after each attack", [] {
        Hero hero;
        hero.addDodgeChancePercent(50, true);
        hero.add(HeroStatus::DodgePrediction);
        hero.gainLevel(noOtherMonsters);
        bool dodgeNext = hero.predictDodgeNext();
        int changes = 0;
        for (int i = 0; i < 1000; ++i)
        {
          // Alternate between level 1 and 2 monsters to change attack order
          Monster monster(MonsterType::MeatMan, Level{1 + i % 2});
          attack(hero, monster);
          if (dodgeNext)
          {
            AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax()));
            AssertThat(hero.has(HeroStatus::DodgePrediction), IsFalse());
            hero.add(HeroStatus::DodgePrediction);
          }
          else
          {
            AssertThat(hero.getHitPoints(), IsLessThan(hero.getHitPointsMax()));
            AssertThat(hero.has(HeroStatus::DodgePrediction), IsTrue());
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
        hero.add(HeroStatus::DodgeTemporary, 100);
        hero.add(HeroStatus::Reflexes);
        Monster monster(MonsterType::MeatMan, Level{2});
        AssertThat(attack(hero, monster), Equals(Summary::Safe));
        AssertThat(monster.getHitPointsMax() - monster.getHitPoints(), Equals(2u * hero.getDamageVersusStandard()));
      });
    });
    describe("Exhausted", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      hero.gainLevel(noOtherMonsters);
      hero.add(HeroTrait::Damned);
      it("should be computed automatically for damned hero", [&] {
        AssertThat(hero.has(HeroStatus::Exhausted), IsFalse());
        hero.loseHitPointsOutsideOfFight(3, noOtherMonsters);
        AssertThat(hero.has(HeroStatus::Exhausted), IsTrue());
      });
      it("should limit health recovery", [&] {
        AssertThat(hero.getHitPoints(), Equals(27u));
        hero.recover(1, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(28u));
      });
      it("should prevent mana recovery", [&] {
        hero.loseManaPoints(1);
        AssertThat(hero.getManaPoints(), Equals(9u));
        hero.recover(2, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(9u));
        AssertThat(hero.has(HeroStatus::Exhausted), IsFalse());
        hero.recover(1, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(10u));
      });
    });
    describe("First Strike", [] {
      it("should give initiative versus regular monsters of any level", [&] {
        Hero hero;
        Monster boss(MonsterType::Goat, Level{10});
        hero.add(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(boss), IsTrue());
      });
      it("should not give initiative versus first-strike monsters of any level", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        Monster goblin(MonsterType::Goblin, Level{1});
        hero.add(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
      });
    });
    describe("Heavy Fireball", [] {
      Hero hero(HeroClass::Wizard, HeroRace::Elf);
      hero.add(HeroStatus::HeavyFireball);
      Monster monster(MonsterType::MeatMan, Level{3});
      it("should increase fireball damage by 4 per caster level", [&] {
        auto hp = monster.getHitPoints();
        AssertThat(cast(hero, monster, Spell::Burndayraz), Equals(Summary::Safe));
        hp -= 8;
        AssertThat(monster.getHitPoints(), Equals(hp));
        AssertThat(monster.getBurnStackSize(), Equals(2u));
        hero.gainLevel(noOtherMonsters);
        hero.gainLevel(noOtherMonsters);
        AssertThat(cast(hero, monster, Spell::Burndayraz), Equals(Summary::Safe));
        hp -= 8 * 3 + 2;
        AssertThat(monster.getHitPoints(), Equals(hp));
        AssertThat(monster.getBurnStackSize(), Equals(6u));
      });
      it("should let monster retaliate", [&] {
        hero.gainLevel(noOtherMonsters);
        monster.recover(15);
        const int retaliateDamage = monster.getDamage() / 2;
        AssertThat(cast(hero, monster, Spell::Burndayraz), Equals(Summary::Safe));
        AssertThat(hero.getHitPointsMax() - hero.getHitPoints(), Equals(retaliateDamage));
        AssertThat(cast(hero, monster, Spell::Burndayraz), Equals(Summary::Win));
        AssertThat(hero.getHitPointsMax() - hero.getHitPoints(), Equals(retaliateDamage));
      });
    });
    describe("Knockback", [] { /* TODO */ });
    describe("Life Steal", [] {
      Hero hero;
      hero.add(HeroStatus::LifeSteal);
      Monster cow("", {Level{2}, 1000_HP, 1_damage}, {}, {});
      it("should heal hero by 1 per hero level and life steal level and allow overheal", [&] {
        attack(hero, cow);
        AssertThat(hero.getHitPoints(), Equals(10u + 1u - 1u));
        hero.add(HeroStatus::LifeSteal);
        attack(hero, cow);
        AssertThat(hero.getHitPoints(), Equals(10u + 2u - 1u));
        hero.gainLevel(noOtherMonsters);
        attack(hero, cow);
        AssertThat(hero.getHitPoints(), Equals(20u + 2u * 2u - 1u));
      });
      it("should be twice as effective against lower level monsters", [&] {
        hero.gainLevel(noOtherMonsters);
        attack(hero, cow);
        const auto steal = 2 * hero.getLevel() * hero.getIntensity(HeroStatus::LifeSteal);
        AssertThat(hero.getHitPoints(), Equals(30u + steal - 1u));
      });
      it("should be applied twice for reflexes", [&] {
        hero.loseHitPointsOutsideOfFight(30, noOtherMonsters);
        const auto healthInitial = hero.getHitPoints();
        hero.add(HeroStatus::Reflexes);
        const auto steal = 2 * hero.getLevel() * hero.getIntensity(HeroStatus::LifeSteal);
        AssertThat(attack(hero, cow), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(healthInitial + 2u * steal - 1u));
      });
      it("should not heal above 150% max. HP", [&] {
        hero.gainLevel(noOtherMonsters);
        hero.add(HeroStatus::LifeSteal);
        hero.changeDamageBonusPercent(30);
        const auto steal = 2 * hero.getLevel() * hero.getIntensity(HeroStatus::LifeSteal);
        AssertThat(steal, IsGreaterThan(hero.getHitPointsMax() / 2u + 1u));
        hero.add(HeroStatus::SlowStrike);
        AssertThat(attack(hero, cow), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() * 3u / 2u));
      });
      it("should be applied immediately after hero's attack", [] {
        Hero hero;
        Monster goblin(MonsterType::Goblin, Level{1});
        hero.add(HeroStatus::LifeSteal);
        hero.loseHitPointsOutsideOfFight(10 - goblin.getDamage(), noOtherMonsters);
        goblin.slow();
        AssertThat(attack(hero, goblin), Equals(Summary::Safe));
        hero.recover(10, noOtherMonsters);
        goblin.recover(10);
        hero.loseHitPointsOutsideOfFight(10 - goblin.getDamage(), noOtherMonsters);
        AssertThat(attack(hero, goblin), Equals(Summary::Death));
      });
      it("should never exceed actual damage done", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        hero.add(HeroStatus::LifeSteal);
        hero.add(HeroStatus::LifeSteal);
        hero.add(HeroStatus::LifeSteal);
        hero.add(HeroStatus::LifeSteal);
        hero.add(HeroStatus::LifeSteal);
        const auto expected = 2 * hero.getLevel() * hero.getIntensity(HeroStatus::LifeSteal);
        const auto limit = hero.getDamageVersusStandard();
        AssertThat(expected > limit, IsTrue());
        hero.loseHitPointsOutsideOfFight(limit, noOtherMonsters);
        Monster monster("", {Level{1}, HitPoints{limit + 1}, 0_damage}, {}, {});
        AssertThat(attack(hero, monster), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax()));
        AssertThat(attack(hero, monster), Equals(Summary::Win));
        AssertThat(hero.getHitPoints(), Equals(hero.getHitPointsMax() + 1));
      });
      it("should not work on bloodless monsters", [] {
        Hero hero;
        hero.add(HeroStatus::LifeSteal);
        Monster monster("Bloodless", {Level{1}, 10_HP, 3_damage}, {}, {MonsterTrait::Bloodless});
        attack(hero, monster);
        AssertThat(hero.getHitPoints(), Equals(7u));
      });
    });
    describe("Magical Attack", [] {
      it("should convert hero's damage into magical damage", [] {
        Hero hero;
        Monster monster(MonsterType::Wraith, Level{2});
        AssertThat(monster.getHitPoints(), Equals(11u));
        AssertThat(attack(hero, monster), Equals(Summary::Safe));
        AssertThat(monster.getHitPoints(), Equals(7u));
        hero.refillHealthAndMana();
        hero.add(HeroStatus::MagicalAttack);
        attack(hero, monster);
        AssertThat(monster.getHitPoints(), Equals(2u));
      });
    });
    describe("Mana Burned", [] {
      it("should prevent mana recovery by uncovering tiles", [] {
        Hero hero;
        hero.add(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(0u));
        hero.recover(10, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(0u));
      });
      it("should allow other means of mana recovery", [] {
        Hero hero(HeroClass::Thief, HeroRace::Human);
        hero.add(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(0u));
        hero.add(HeroStatus::Schadenfreude);
        Monster monster(MonsterType::MeatMan, Level{1});
        attack(hero, monster);
        AssertThat(hero.getManaPoints(), Equals(monster.getDamage()));
        hero.use(Potion::HealthPotion, noOtherMonsters);
        AssertThat(hero.getManaPoints(), Equals(monster.getDamage() + 2));
      });
    });
    describe("Mana Burn Immune", [] {
      it("should prevent being mana burned", [] {
        Hero hero;
        hero.add(HeroStatus::ManaBurnImmune);
        hero.add(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero.has(HeroDebuff::ManaBurned), Equals(false));
        Hero hero2;
        hero2.add(HeroDebuff::ManaBurned, noOtherMonsters);
        AssertThat(hero2.has(HeroDebuff::ManaBurned), Equals(true));
        hero2.add(HeroStatus::ManaBurnImmune);
        AssertThat(hero2.has(HeroDebuff::ManaBurned), Equals(false));
      });
    });
    describe("Might", [] {
      Hero hero;
      it("should increase damage of next attack by 30%", [&] {
        hero.add(HeroStatus::Might);
        AssertThat(hero.getDamageBonusPercent(), Equals(30));
      });
      it("should lower enemies resistances by 3%", [&] {
        Monster monster("", {Level{1}, 10_HP, 1_damage}, {50_physicalresist, 75_magicalresist}, {});
        attack(hero, monster);
        AssertThat(monster.getPhysicalResistPercent(), Equals(47u));
        AssertThat(monster.getMagicalResistPercent(), Equals(72u));
      });
      it("should wear off", [&] {
        AssertThat(hero.has(HeroStatus::Might), IsFalse());
        AssertThat(hero.getDamageBonusPercent(), Equals(0));
      });
    });
    describe("Pierce Physical", [] {
      it("should ignore 35% of physical resist", [] {
        Hero hero;
        hero.setHitPointsMax(100);
        hero.gainLevel(noOtherMonsters);
        hero.gainLevel(noOtherMonsters);
        hero.gainLevel(noOtherMonsters);
        const auto damage = hero.getDamageVersusStandard();

        Monster resist25(MonsterType::SteelGolem, Level{5}); // 25% physical resist
        AssertThat(attack(hero, resist25), Equals(Summary::Safe));
        const unsigned resisted = damage / 4;
        AssertThat(resist25.getHitPointsMax() - resist25.getHitPoints(), Equals(damage - resisted));
        resist25.recover(100);

        AssertThat(hero.receive(BlacksmithItem::ReallyBigSword), IsTrue());
        AssertThat(hero.has(HeroStatus::PiercePhysical), IsTrue());
        AssertThat(attack(hero, resist25), Equals(Summary::Safe));
        AssertThat(resist25.getHitPointsMax() - resist25.getHitPoints(), Equals(damage));

        Monster resist50(MonsterType::GooBlob, Level{3});
        AssertThat(attack(hero, resist50), Equals(Summary::Safe));
        const unsigned resistedReduced = damage * (50 - 35) / 100;
        AssertThat(resist50.getHitPointsMax() - resist50.getHitPoints(), Equals(damage - resistedReduced));

        Monster resist0(MonsterType::MeatMan, Level{3});
        AssertThat(attack(hero, resist0), Equals(Summary::Safe));
        AssertThat(resist0.getHitPointsMax() - resist0.getHitPoints(), Equals(damage));

        hero.convert(BlacksmithItem::ReallyBigSword, noOtherMonsters);
        AssertThat(hero.has(HeroStatus::PiercePhysical), IsFalse());
      });
      it("should not break physical immunity", [] {
        Hero hero;
        AssertThat(hero.receive(BlacksmithItem::ReallyBigSword), IsTrue());
        Monster immune("Immune", {Level{1}, 10_HP, 5_damage}, {100_physicalresist}, {});
        AssertThat(attack(hero, immune), Equals(Summary::Safe));
        AssertThat(immune.getHitPoints(), Equals(10u));
      });
      it("should not affect magical attacks", [] {
        Hero hero;
        hero.setHitPointsMax(100);
        hero.gainLevel(noOtherMonsters);
        hero.gainLevel(noOtherMonsters);
        hero.add(HeroStatus::ConsecratedStrike);
        AssertThat(hero.receive(BlacksmithItem::ReallyBigSword), IsTrue());
        Monster troll(MonsterType::FrozenTroll, Level{3});
        AssertThat(troll.getPhysicalResistPercent(), Equals(50u));
        AssertThat(troll.getMagicalResistPercent(), Equals(50u));
        const auto damage = hero.getDamageVersusStandard();
        const unsigned resisted = damage / 2;
        AssertThat(attack(hero, troll), Equals(Summary::Safe));
        AssertThat(troll.getHitPointsMax() - troll.getHitPoints(), Equals(damage - resisted));
        troll.recover(100);
        AssertThat(hero.has(HeroStatus::ConsecratedStrike), IsFalse());
        hero.add(HeroStatus::MagicalAttack);
        AssertThat(attack(hero, troll), Equals(Summary::Safe));
        AssertThat(troll.getHitPointsMax() - troll.getHitPoints(), Equals(damage - resisted));
        troll.recover(100);
        hero.reset(HeroStatus::MagicalAttack);
        const unsigned resistedReduced = damage * (50 - 35) / 100;
        AssertThat(attack(hero, troll), Equals(Summary::Safe));
        AssertThat(troll.getHitPointsMax() - troll.getHitPoints(), Equals(damage - resistedReduced));
      });
    });
    describe("Poisoned", [] {
      it("should prevent health recovery", [] {
        Hero hero;
        hero.loseHitPointsOutsideOfFight(9, noOtherMonsters);
        hero.recover(1, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(2u));
        hero.add(HeroDebuff::Poisoned, noOtherMonsters);
        hero.recover(10, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(2u));
      });
    });
    describe("Poisonous", [] {
      it("should poison the monster (1 point per hero level)", [] {
        Hero hero({100_HP, 0_MP, 1_damage}, {}, Experience{5});
        hero.add(HeroStatus::Poisonous, 3);
        Monster monster(MonsterType::GooBlob, Level{4});
        AssertThat(attack(hero, monster), Equals(Summary::Safe));
        AssertThat(monster.getPoisonAmount(), Equals(15u));
      });
    });
    describe("Poison Immune", [] {
      it("should prevent being poisoned", [] {
        Hero hero;
        hero.add(HeroStatus::PoisonImmune);
        hero.add(HeroDebuff::Poisoned, noOtherMonsters);
        AssertThat(hero.has(HeroDebuff::Poisoned), Equals(false));
        Hero hero2;
        hero2.add(HeroDebuff::Poisoned, noOtherMonsters);
        AssertThat(hero2.has(HeroDebuff::Poisoned), Equals(true));
        hero2.add(HeroStatus::PoisonImmune);
        AssertThat(hero2.has(HeroDebuff::Poisoned), Equals(false));
      });
    });
    describe("Reflexes", [] {
      it("should cause 2 hits", [] {
        Hero hero;
        hero.add(HeroStatus::Reflexes);
        Monster monster(1, 2 * hero.getDamageVersusStandard(), 1);
        AssertThat(attack(hero, monster), Equals(Summary::Win));
      });
    });
    describe("Sanguine", [] { /* TODO */ });
    describe("Schadenfreude", [] {
      it("should refill mana equal to damage received", [] {
        Hero hero;
        hero.loseManaPoints(10);
        hero.add(HeroStatus::Schadenfreude);
        Monster monster("", {Level{1}, 6_HP, 8_damage}, {}, {});
        attack(hero, monster);
        AssertThat(hero.getManaPoints(), Equals(8u));
        AssertThat(hero.getHitPoints(), Equals(2u));
        AssertThat(hero.has(HeroStatus::Schadenfreude), IsFalse());
      });
    });
    describe("Slow Strike", [] {
      Hero hero({}, {}, Experience(10));
      Monster meatMan(MonsterType::MeatMan, Level{1});
      Monster goblin(MonsterType::Goblin, Level{1});
      it("should give initiative to regular monsters of any level", [&] {
        hero.add(HeroStatus::SlowStrike);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsFalse());
      });
      it("should cancel First Strike", [&] {
        hero.add(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsTrue());
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
        hero.reset(HeroStatus::FirstStrikeTemporary);
      });
      it("should give initiative to slowed monsters of any level (regardless of monster's first strike)", [&] {
        meatMan.slow();
        goblin.slow();
        AssertThat(hero.hasInitiativeVersus(meatMan), IsFalse());
        AssertThat(hero.hasInitiativeVersus(goblin), IsFalse());
      });
      it("should not give initiative to slowed monsters when cancelled by First Strike", [&] {
        hero.add(HeroStatus::FirstStrikeTemporary);
        AssertThat(hero.hasInitiativeVersus(meatMan), IsTrue());
        AssertThat(hero.hasInitiativeVersus(goblin), IsTrue());
      });
    });
    describe("Spirit Strength", [] {
      Hero hero;
      it("should add base damage for the next attack", [&] {
        AssertThat(hero.getBaseDamage(), Equals(5u));
        hero.add(HeroStatus::SpiritStrength, 7);
        AssertThat(hero.getBaseDamage(), Equals(12u));
        hero.changeDamageBonusPercent(100);
        AssertThat(hero.getDamageVersusStandard(), Equals(24u));
      });
      it("should wear off after the next physical attack", [&] {
        Monster monster("", {Level{2}, 40_HP, 1_damage}, {}, {});
        hero.add(HeroStatus::Reflexes);
        AssertThat(cast(hero, monster, Spell::Burndayraz), Equals(Summary::Safe));
        AssertThat(hero.has(HeroStatus::SpiritStrength), IsTrue());
        AssertThat(monster.getHitPoints(), Equals(36u));
        AssertThat(monster.getBurnStackSize(), Equals(1u));
        AssertThat(attack(hero, monster), Equals(Summary::Safe));
        const unsigned expected = 36 - 1 /* burn stack */ - 24 /* spirit attack */ - 10 /* regular attack */;
        AssertThat(monster.getHitPoints(), Equals(expected));
        AssertThat(hero.has(HeroStatus::SpiritStrength), IsFalse());
      });
      it("should wear off after casting Pisorf", [] {
        Hero hero(HeroClass::Transmuter, HeroRace::Human);
        Monster monster("", {Level{1}, 31_HP, 1_damage}, {}, {});
        hero.addConversionPoints(100, noOtherMonsters);
        AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(11u));
        AssertThat(hero.getBaseDamage(), Equals(16u));
        hero.changeBaseDamage(34);
        AssertThat(hero.getBaseDamage(), Equals(50u));
        hero.use(Potion::ManaPotion, noOtherMonsters);
        resources.numWalls += 1;
        AssertThat(cast(hero, monster, Spell::Pisorf), Equals(Summary::Safe));
        AssertThat(hero.has(HeroStatus::SpiritStrength), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1u /* 31 - 60% * 50 */));
      });
      it("should not pile up", [] {
        Hero hero(HeroClass::Transmuter, HeroRace::Human);
        hero.addConversionPoints(100, noOtherMonsters);
        AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(11u));
        hero.addConversionPoints(100, noOtherMonsters);
        AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(11u));
        hero.recover(10, noOtherMonsters);
        hero.use(Potion::StrengthPotion, noOtherMonsters);
        AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(11u));
        hero.use(Potion::StrengthPotion, noOtherMonsters);
        AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(11u));
      });
    });
    describe("Stone Skin", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      it("should add 20% physical resistance per stack", [&] {
        hero.add(HeroStatus::StoneSkin);
        AssertThat(hero.getPhysicalResistPercent(), Equals(20));
        hero.add(HeroStatus::StoneSkin, 2);
        AssertThat(hero.getPhysicalResistPercent(), Equals(60));
        hero.add(HeroStatus::StoneSkin);
        AssertThat(hero.getPhysicalResistPercent(), Equals(65));
      });
      it("should wear off when hit", [&] {
        Monster monster(MonsterType::MeatMan, Level{1});
        AssertThat(attack(hero, monster), Equals(Summary::Safe));
        AssertThat(hero.getHitPoints(), Equals(19u));
        AssertThat(hero.has(HeroStatus::StoneSkin), IsFalse());
        hero.add(HeroStatus::StoneSkin);
        AssertThat(attack(hero, monster), Equals(Summary::Win));
        AssertThat(hero.has(HeroStatus::StoneSkin), IsTrue());
      });
    });
    describe("Weakened", [] {
      it("should reduce base damage by one per stack level", [] {
        Hero hero;
        hero.changeDamageBonusPercent(100);
        AssertThat(hero.getDamageBonusPercent(), Equals(100));
        const auto base_initial = hero.getBaseDamage();
        const auto damage_initial = hero.getDamageVersusStandard();
        hero.add(HeroDebuff::Weakened, noOtherMonsters);
        AssertThat(hero.getBaseDamage(), Equals(base_initial - 1u));
        AssertThat(hero.getDamageVersusStandard(), Equals(damage_initial - 2u));
        AssertThat(hero.getDamageBonusPercent(), Equals(100));
        hero.add(HeroDebuff::Weakened, noOtherMonsters);
        AssertThat(hero.getBaseDamage(), Equals(base_initial - 2u));
      });
      it("should not reduce damage below zero", [] {
        Hero hero;
        hero.add(HeroDebuff::Weakened, noOtherMonsters, static_cast<int>(hero.getBaseDamage()) + 1);
        AssertThat(hero.getBaseDamage(), Is().Not().LessThan(0));
        AssertThat(hero.getDamageVersusStandard(), Is().Not().LessThan(0));
      });
    });
  });
}
