#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/Hero.hpp"
#include "engine/Magic.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
}

void testCombatInitiative()
{
  describe("Initiative", [] {
    Hero hero;
    Monster boss(10, 10, 3);
    Monster monster(1, 10, 3);
    it("should go to the monster of higher level", [&] { AssertThat(hero.hasInitiativeVersus(boss), IsFalse()); });
    it("should go to the monster of equal level", [&] { AssertThat(hero.hasInitiativeVersus(monster), IsFalse()); });
    it("should go to the hero if he has higher level", [&] {
      hero.gainLevel(noOtherMonsters);
      AssertThat(hero.hasInitiativeVersus(monster), IsTrue());
    });
  });
}

void testMelee()
{
  describe("Melee simulation", [] {
    Hero hero;
    Monster monster(3, 15, 5);
    it("should correctly predict safe outcome (simple case)",
       [&] { AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe)); });
    it("should correctly predict death outcome (simple case)",
       [&] { AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Death)); });
    it("should correctly predict win outcome (one shot, monster has lower level)", [&] {
      Hero hero;
      hero.gainExperienceNoBonuses(30, noOtherMonsters);
      AssertThat(hero.getLevel(), Equals(4u));
      AssertThat(hero.getXP(), Equals(0u));
      hero.loseHitPointsOutsideOfFight(hero.getHitPointsMax() - 1, noOtherMonsters);
      monster.recover(100);
      AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Win));
    });
    it("should correctly predict hitpoint loss (simple case)", [] {
      Hero hero;
      Monster monster(3, 15, 9);
      AssertThat(hero.getHitPoints(), Equals(10u));
      AssertThat(monster.getHitPoints(), Equals(15u));
      Combat::attack(hero, monster, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(10u - monster.getDamage()));
      AssertThat(monster.getHitPoints(), Equals(15u - hero.getDamageOutputVersus(monster)));
    });
  });

  describe("Monster death gaze", [] {
    class CustomMonsterTraits : public MonsterTraits
    {
    public:
      static MonsterTraits withDeathGaze(unsigned char percent)
      {
        CustomMonsterTraits traits;
        traits.deathGazePercent = percent;
        return static_cast<MonsterTraits>(traits);
      }
    };
    it("should petrify hero with low health (<50%)", [] {
      Hero hero;
      Monster gorgon(MonsterType::Gorgon, 1);
      AssertThat(Combat::attack(hero, gorgon, noOtherMonsters), Equals(Summary::Win));
      hero.loseHitPointsOutsideOfFight(3, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(4));
      Monster gorgon2(MonsterType::Gorgon, 1);
      AssertThat(Combat::attack(hero, gorgon2, noOtherMonsters), Equals(Summary::Petrified));
    });
    it("should be available with 100% intensity", [] {
      Hero hero;
      auto traits = CustomMonsterTraits::withDeathGaze(100);
      Monster monster("", {Level{1}, 100_HP, 1_damage}, {}, std::move(traits));
      hero.add(HeroStatus::DeathProtection);
      AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
      AssertThat(hero.has(HeroStatus::DeathProtection), IsTrue());
      hero.recover(100);
      hero.loseHitPointsOutsideOfFight(1, noOtherMonsters);
      AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
      AssertThat(hero.has(HeroStatus::DeathProtection), IsFalse());
      hero.recover(100);
      hero.loseHitPointsOutsideOfFight(1, noOtherMonsters);
      AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Petrified));
    });
    it("should be available with 101% intensity", [] {
      Hero hero;
      MonsterTraits traits = CustomMonsterTraits::withDeathGaze(101);
      Monster monster("", {Level{1}, 10_HP, 1_damage}, {}, std::move(traits));
      AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Petrified));
    });
  });
}

void testCombatWithTwoMonsters()
{
  describe("Burn stack damage", [] {
    Hero hero;
    Monsters allMonsters;
    SimpleResources resources;
    allMonsters.reserve(2); // prevent reallocation
    Monster& burning = allMonsters.emplace_back(Monster(1, 10, 1));
    Monster& nextTarget = allMonsters.emplace_back(MonsterType::MeatMan, 1);
    it("should occur on physical attack to other monster", [&] {
      AssertThat(Magic::cast(hero, burning, Spell::Burndayraz, allMonsters, resources), Equals(Summary::Safe));
      AssertThat(burning.getHitPoints(), Equals(6u));
      AssertThat(Combat::attack(hero, nextTarget, allMonsters), Equals(Summary::Safe));
      AssertThat(burning.isBurning(), IsFalse());
      AssertThat(burning.getHitPoints(), Equals(5u));
    });
    it("should count as a burning kill", [&] {
      hero.recoverManaPoints(2);
      AssertThat(Magic::cast(hero, burning, Spell::Burndayraz, allMonsters, resources), Equals(Summary::Safe));
      AssertThat(burning.getHitPoints(), Equals(1u));
      hero.followDeity(God::GlowingGuardian, 0);
      AssertThat(hero.getPiety(), Equals(5u));
      AssertThat(Combat::attack(hero, nextTarget, allMonsters), Equals(Summary::Safe));
      AssertThat(burning.isDefeated(), IsTrue());
      AssertThat(burning.isBurning(), IsFalse());
      AssertThat(burning.getHitPoints(), Equals(0u));
      AssertThat(hero.getXP(), Equals(1u));
      AssertThat(hero.getPiety(), Equals(6u));
    });
  });
}

void testCombat()
{
  testCombatInitiative();
  testMelee();
  testCombatWithTwoMonsters();
}
