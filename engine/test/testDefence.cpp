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
      auto monster = Monster{{Level{1}, 10_HP, 1_damage}, {50_physicalresist}};
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
      auto monster = Monster{{Level{1}, 10_HP, 1_damage}, {75_magicalresist}};
      it("should be applied for magical damage, resisted damage is rounded down", [&] {
        AssertThat(monster.getMagicalResistPercent(), Equals(75u));
        monster.takeDamage(5 * 4 + 1, DamageType::Magical);
        AssertThat(monster.getHitPoints(), Equals(10u - 5u - 1u));
      });
    });
    describe("Corroded", [] {
      it("should add 1 damage point per level", [] {
        Hero hero;
        auto monster = Monster{{Level{1}, 20_HP, 1_damage}};
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
        auto monster = Monster{{Level{1}, 10_HP, 1_damage}, {50_physicalresist, 75_magicalresist}};
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
        auto monster = Monster{{Level{1}, 10_HP, 10_damage, 2_dprot}, {2_physicalresist, 3_magicalresist}};
        monster.erodeResitances();
        AssertThat(monster.getPhysicalResistPercent(), Equals(0u));
        AssertThat(monster.getMagicalResistPercent(), Equals(0u));
        auto monster2 = Monster{{Level{1}, 10_HP, 1_damage}, {3_physicalresist, 4_magicalresist}};
        monster2.erodeResitances();
        AssertThat(monster2.getPhysicalResistPercent(), Equals(0u));
        AssertThat(monster2.getMagicalResistPercent(), Equals(1u));
      });
    });
    describe("Death protection", [] {
      auto monster = Monster{{Level{1}, 10_HP, 10_damage, 2_dprot}};
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
        hero.add(HeroStatus::DamageReduction);
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8u));
        hero.add(HeroStatus::DamageReduction, 99);
        hero.takeDamage(90, DamageType::Physical, noOtherMonsters);
        hero.takeDamage(99, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8u));
        hero.takeDamage(104, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(4u));
        hero.add(HeroStatus::DeathProtection);
        hero.takeDamage(104, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(1u));
        AssertThat(hero.has(HeroStatus::DeathProtection), IsFalse());
        hero.takeDamage(101, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.isDefeated(), IsTrue());
      });
    });
    describe("Corroded", [] {
      it("should increase damage taken by 1 per stack size", [] {
        Hero hero;
        hero.add(HeroDebuff::Corroded, noOtherMonsters);
        hero.takeDamage(1, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8u));
        hero.takeDamage(1, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(6u));
        hero.add(HeroDebuff::Corroded, noOtherMonsters);
        hero.add(HeroDebuff::Corroded, noOtherMonsters);
        hero.takeDamage(3, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.isDefeated(), IsTrue());
      });
      it("should only cause extra damage if any damage was taken at all", [] {
        Hero hero;
        hero.add(HeroDebuff::Corroded, noOtherMonsters);
        hero.add(HeroStatus::DamageReduction);
        hero.takeDamage(1, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10u));
      });
    });
    describe("Damage reduction, resistance and corrosion", [] {
      it("should be applied in this order", [] {
        Hero hero;
        hero.setPhysicalResistPercent(50);
        hero.add(HeroStatus::DamageReduction, 2);
        hero.add(HeroDebuff::Corroded, noOtherMonsters, 5);
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
        const unsigned expectedDamage = (3 - 2) - 0 /* 50% of 1, rounded down */ + 5;
        AssertThat(hero.getHitPoints(), Equals(10u - expectedDamage));
      });
    });
  });
}
