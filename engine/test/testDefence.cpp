#include "bandit/bandit.h"

#include "Hero.hpp"
#include "Monster.hpp"

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
        AssertThat(monster.getPhysicalResistPercent(), Equals(50));
        monster.takeDamage(10, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(5));
      });
      it("should be rounded down", [&] {
        monster.takeDamage(1, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(4));
      });
    });
    describe("Magical resistance", [] {
      Monster monster("", MonsterStats{1, 10, 1, 0}, Defence{0, 75}, {});
      it("should be applied for magical damage, resisted damage is rounded down", [&] {
        AssertThat(monster.getMagicalResistPercent(), Equals(75));
        monster.takeDamage(5 * 4 + 1, DamageType::Magical);
        AssertThat(monster.getHitPoints(), Equals(10 - 5 - 1));
      });
    });
    describe("Corroded", [] {
      it("should add 1 damage point per level", [] {
        Hero hero;
        Monster monster(1, 20, 1);
        monster.corrode();
        monster.takeDamage(1, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(18));
        monster.corrode();
        monster.takeDamage(1, DamageType::Magical);
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
        monster.takeDamage(2, DamageType::Physical);
        AssertThat(monster.getHitPoints(), Equals(8));
      });
    });
    describe("Death protection", [] {
      Monster monster("", MonsterStats{1, 10, 10, 2}, {}, {});
      it("should prevent defeat", [&] {
        monster.takeDamage(100, DamageType::Physical);
        AssertThat(monster.isDefeated(), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1));
      });
      it("should wear off", [&] {
        monster.takeDamage(100, DamageType::Physical);
        AssertThat(monster.isDefeated(), IsFalse());
        AssertThat(monster.getHitPoints(), Equals(1));
        AssertThat(monster.getDeathProtection(), Equals(0));
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
        AssertThat(hero.getHitPoints(), Equals(10 - 6));
      });
      it("should not block magical damage", [&] {
        AssertThat(hero.getHitPoints(), Equals(4));
        hero.takeDamage(3, DamageType::Magical, noOtherMonsters);
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
        hero.takeDamage(11, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(10 - 6));
      });
      it("should not block physical damage", [&] {
        AssertThat(hero.getHitPoints(), Equals(4));
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
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
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8));
        hero.addStatus(HeroStatus::DamageReduction, 99);
        hero.takeDamage(90, DamageType::Physical, noOtherMonsters);
        hero.takeDamage(99, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(8));
        hero.takeDamage(104, DamageType::Physical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(4));
        hero.addStatus(HeroStatus::DeathProtection);
        hero.takeDamage(104, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(1));
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
        AssertThat(hero.getHitPoints(), Equals(8));
        hero.takeDamage(1, DamageType::Magical, noOtherMonsters);
        AssertThat(hero.getHitPoints(), Equals(6));
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
        AssertThat(hero.getHitPoints(), Equals(10));
      });
    });
    describe("Damage reduction, resistance and corrosion", [] {
      it("should be applied in this order", [] {
        Hero hero;
        hero.setPhysicalResistPercent(50);
        hero.addStatus(HeroStatus::DamageReduction, 2);
        hero.addStatus(HeroDebuff::Corroded, noOtherMonsters, 5);
        hero.takeDamage(3, DamageType::Physical, noOtherMonsters);
        const int expectedDamage = (3 - 2) - 0 /* 50% of 1, rounded down */ + 5;
        AssertThat(hero.getHitPoints(), Equals(10 - expectedDamage));
      });
    });
  });
}
