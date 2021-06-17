#include "bandit/bandit.h"

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
}

void testDefenceBasics()
{
  describe("Monster", [] {
    describe("Physical resistance", [] {
      Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{50, 0}, {});
      it("should reduce damage according to resistance %", [&] {
        AssertThat(monster.getPhysicalResistPercent(), Equals(50u));
        monster.takeDamage(10, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(5u));
      });
      it("should be rounded down", [&] {
        monster.takeDamage(1, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(4u));
      });
    });
    describe("Magical resistance", [] {
      Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{0, 75}, {});
      it("should be applied for magical damage, resisted damage is rounded down", [&] {
        AssertThat(monster.getMagicalResistPercent(), Equals(75u));
        monster.takeDamage(5 * 4 + 1, DamageType::Magical);
        AssertThat(monster.getHitPoints(), Equals(10u - 5u - 1u));
      });
    });
    describe("Corroded", [] {
      it("should add 1 damage point per level", [] {
        Hero hero;
        Monster monster(1, 20, 1);
        monster.corrode();
        monster.takeDamage(1, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(18u));
        monster.corrode();
        monster.takeDamage(1, DamageType::Magical);
        AssertThat(monster.getHitPoints(), Equals(15u));
      });
    });
    describe("Resistance crushing", [] {
      it("should reduce resistances by 3 percentage points", [] {
        Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{50, 75}, {});
        monster.erodeResitances();
        AssertThat(monster.getPhysicalResistPercent(), Equals(47u));
        AssertThat(monster.getMagicalResistPercent(), Equals(72u));
        monster.erodeResitances();
        monster.erodeResitances();
        monster.erodeResitances();
        monster.erodeResitances();
        AssertThat(monster.getPhysicalResistPercent(), Equals(47u - 12u));
        AssertThat(monster.getMagicalResistPercent(), Equals(72u - 12u));
        // 35% physical resistance remaining -> no damage reduction
        monster.takeDamage(2, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(8u));
      });
      it("should not reduce resistances below 0", [] {
        Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{2, 3}, {});
        monster.erodeResitances();
        AssertThat(monster.getPhysicalResistPercent(), Equals(0u));
        AssertThat(monster.getMagicalResistPercent(), Equals(0u));
        Monster monster2("", MonsterStats{1, 10, 1, 0}, Defence{3, 4}, {});
        monster2.erodeResitances();
        AssertThat(monster2.getPhysicalResistPercent(), Equals(0u));
        AssertThat(monster2.getMagicalResistPercent(), Equals(1u));
      });
    });
    describe("Death protection", [] {
      Monster monster("", MonsterStats{1, 10, 10, 2}, {}, {});
      it("should prevent defeat", [&] {
        monster.takeDamage(100, DamageType::Physical);
        AssertThat(monster.isDefeated(), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1u));
      });
      it("should wear off", [&] {
        monster.takeDamage(100, DamageType::Physical);
        AssertThat(monster.isDefeated(), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1u));
        AssertThat(monster.getDeathProtection(), Equals(0u));
        monster.takeDamage(1, DamageType::Physical);
        AssertThat(monster.isDefeated(), IsTrue());
      });
    });
  });

  describe("Hero", [] {
    describe("Physical resistance", [] {
      Hero hero;
      hero.setPhysicalResistPercent(50);
      it("should be accounted for (rounding down)", [&] {
        hero.takeDamage(11, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10u - 6u));
      });
      it("should not block magical damage", [&] {
        AssertThat(hero.getHitPoints(), Equals(4u));
        hero.takeDamage(3, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(1u));
      });
      it("should be capped (at 65% by default)", [] {
        Hero hero;
        hero.setPhysicalResistPercent(100);
        hero.changePhysicalResistPercent(+100);
        AssertThat(hero.getPhysicalResistPercent(), Equals(65));
        Hero monk(HeroClass::Monk);
        monk.changePhysicalResistPercent(+100);
        AssertThat(monk.getPhysicalResistPercent(), Equals(75));
      });
      it("should be non-negative", [] {
        Hero hero;
        hero.setPhysicalResistPercent(-100);
        AssertThat(hero.getPhysicalResistPercent(), Equals(0));
        hero.changePhysicalResistPercent(-1);
        AssertThat(hero.getPhysicalResistPercent(), Equals(0));
      });
    });
    describe("Magical resistance", [] {
      Hero hero;
      hero.setMagicalResistPercent(50);
      it("should be accounted for (rounding down)", [&] {
        hero.takeDamage(11, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10u - 6u));
      });
      it("should not block physical damage", [&] {
        AssertThat(hero.getHitPoints(), Equals(4u));
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(1u));
      });
      it("should be capped (at 65% by default)", [] {
        Hero hero;
        hero.setMagicalResistPercent(100);
        hero.changeMagicalResistPercent(+100);
        AssertThat(hero.getMagicalResistPercent(), Equals(65));
      });
      it("should be non-negative", [] {
        Hero hero;
        hero.setMagicalResistPercent(-20);
        AssertThat(hero.getMagicalResistPercent(), Equals(0));
        hero.changeMagicalResistPercent(-10);
        AssertThat(hero.getMagicalResistPercent(), Equals(0));
      });
    });
    describe("Damage reduction", [] {
      it("should reduce incoming damage", [&] {
        Hero hero;
        hero.addStatus(HeroStatus::DamageReduction);
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8u));
        hero.addStatus(HeroStatus::DamageReduction, 99);
        hero.takeDamage(90, DamageType::Physical, noOtherMonsters);
        hero.takeDamage(99, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8u));
        hero.takeDamage(104, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(4u));
        hero.addStatus(HeroStatus::DeathProtection);
        hero.takeDamage(104, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(1u));
        AssertThat(hero.hasStatus(HeroStatus::DeathProtection), IsFalse());
        hero.takeDamage(101, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.isDefeated(), IsTrue());
      });
    });
    describe("Corroded", [] {
      it("should increase damage taken by 1 per stack size", [] {
        Hero hero;
        hero.addStatus(HeroDebuff::Corroded, noOtherMonsters);
        hero.takeDamage(1, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8u));
        hero.takeDamage(1, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(6u));
        hero.addStatus(HeroDebuff::Corroded, noOtherMonsters);
        hero.addStatus(HeroDebuff::Corroded, noOtherMonsters);
        hero.takeDamage(3, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.isDefeated(), IsTrue());
      });
      it("should only cause extra damage if any damage was taken at all", [] {
        Hero hero;
        hero.addStatus(HeroDebuff::Corroded, noOtherMonsters);
        hero.addStatus(HeroStatus::DamageReduction);
        hero.takeDamage(1, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10u));
      });
    });
    describe("Damage reduction, resistance and corrosion", [] {
      it("should be applied in this order", [] {
        Hero hero;
        hero.setPhysicalResistPercent(50);
        hero.addStatus(HeroStatus::DamageReduction, 2);
        hero.addStatus(HeroDebuff::Corroded, noOtherMonsters, 5);
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
        const unsigned expectedDamage = (3 - 2) - 0 /* 50% of 1, rounded down */ + 5;
        AssertThat(hero.getHitPoints(), Equals(10u - expectedDamage));
      });
    });
  });
}
