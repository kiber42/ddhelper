#include "bandit/bandit.h"

#include "engine/Monster.hpp"

using namespace bandit;
using namespace snowhouse;

void testMonsterBasics()
{
  describe("Monster", [] {
    Monster monster(2, 10, 3);
    it("used for test should have level 2 and 10 HP", [&] {
      AssertThat(monster.getLevel(), Equals(2u));
      AssertThat(monster.getHitPoints(), Equals(10u));
    });
    it("with 10 HP should survive a hit with 9 damage points and has 1 HP remaining", [&] {
      monster.takeDamage(9, DamageType::Physical);
      AssertThat(monster.isDefeated(), IsFalse());
      AssertThat(monster.getHitPoints(), Equals(1u));
    });
    it("at level 2 should recover at a rate of 2 HP per explored square", [&] {
      monster.recover(4);
      AssertThat(monster.getHitPoints(), Equals(9u));
    });
    it("should not recover beyond its max HP", [&] {
      monster.recover(10);
      AssertThat(monster.getHitPoints(), Equals(monster.getHitPointsMax()));
    });
    it("should not recover HP while poisoned", [&] {
      monster.takeDamage(1, DamageType::Physical);
      monster.poison(3);
      monster.recover(1);
      AssertThat(monster.getHitPoints(), Equals(9u));
      AssertThat(monster.getPoisonAmount(), Equals(1u));
    });
    it("should reduce poison as it would usually recover HP", [&] {
      monster.recover(1);
      AssertThat(monster.getHitPoints(), Equals(10u));
      AssertThat(monster.isPoisoned(), IsFalse());
    });
    it("should lose 4 HP per caster level when hit by a fireball", [&] {
      monster.takeFireballDamage(2);
      AssertThat(monster.getHitPoints(), Equals(10u - 2u * 4u));
    });
    it("should be burning after hit by a fireball", [&] {
      AssertThat(monster.isBurning(), IsTrue());
      AssertThat(monster.getBurnStackSize(), Equals(1u));
    });
    it("should recover HP at a rate reduced by 1 when burning", [&] {
      monster.recover(4);
      AssertThat(monster.getHitPoints(), Equals(6u));
    });
    it("should take additional fireball damage when already burning", [&] {
      monster.takeFireballDamage(1);
      AssertThat(monster.getHitPoints(), Equals(6u - 1u * 4u - 1u));
      AssertThat(monster.getBurnStackSize(), Equals(2u));
    });
    it("should recover HP at a rate reduced by 1 when burning, independent of burn stack size", [&] {
      monster.recover(5);
      AssertThat(monster.getHitPoints(), Equals(6u));
    });
    it("should take additional fireball damage per burn stack", [&] {
      monster.recover(10);
      monster.takeFireballDamage(1);
      AssertThat(monster.getHitPoints(), Equals(10u - 1u * 4u - 2u));
    });
    it("should not have a burn stack size higher than twice the caster's level",
       [&] { AssertThat(monster.getBurnStackSize(), Equals(2u)); });
    it("should stop burning upon any physical damage, and take damage equal to burn stack size", [&] {
      AssertThat(monster.getHitPoints() - monster.getBurnStackSize(), Equals(2u));
      monster.takeDamage(0, DamageType::Physical);
      AssertThat(monster.isBurning(), IsFalse());
      AssertThat(monster.getHitPoints(), Equals(2u));
    });
    it("should recover from being slowed when taking damage", [&] {
      monster.slow();
      AssertThat(monster.isSlowed(), IsTrue());
      monster.takeDamage(1, DamageType::Physical);
      AssertThat(monster.isSlowed(), IsFalse());
    });
  });
}
