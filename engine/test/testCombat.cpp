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
  SimpleResources resources;

  auto attack(Hero& hero, Monster& monster) { return Combat::attack(hero, monster, noOtherMonsters, resources); }

  auto attack(Hero& hero, Monster& monster, Monsters& monsters, Resources& resources)
  {
    return Combat::attack(hero, monster, monsters, resources);
  }
} // namespace

void testCombatInitiative()
{
  describe("Initiative", [] {
    Hero hero;
    auto boss = Monster{{Level{10}, 10_HP, 3_damage}};
    auto monster = Monster{{Level{1}, 10_HP, 3_damage}};
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
    auto monster = Monster{{Level{3}, 15_HP, 5_damage}};
    it("should correctly predict safe outcome (simple case)",
       [&] { AssertThat(attack(hero, monster), Equals(Summary::Safe)); });
    it("should correctly predict death outcome (simple case)",
       [&] { AssertThat(attack(hero, monster), Equals(Summary::Death)); });
    it("should correctly predict win outcome (one shot, monster has lower level)", [&] {
      Hero hero;
      hero.gainExperienceNoBonuses(30, noOtherMonsters);
      AssertThat(hero.getLevel(), Equals(4u));
      AssertThat(hero.getXP(), Equals(0u));
      hero.loseHitPointsOutsideOfFight(hero.getHitPointsMax() - 1, noOtherMonsters);
      monster.recover(100);
      AssertThat(attack(hero, monster), Equals(Summary::Win));
    });
    it("should correctly predict hitpoint loss (simple case)", [] {
      Hero hero;
      auto monster = Monster{{Level{3}, 15_HP, 9_damage}};
      AssertThat(hero.getHitPoints(), Equals(10u));
      AssertThat(monster.getHitPoints(), Equals(15u));
      attack(hero, monster);
      AssertThat(hero.getHitPoints(), Equals(10u - monster.getDamage()));
      AssertThat(monster.getHitPoints(), Equals(15u - hero.getDamageOutputVersus(monster)));
    });
  });
  describe("Side effects", [] {
    auto traits = MonsterTraits{MonsterTrait::Corrosive, MonsterTrait::CurseBearer, MonsterTrait::ManaBurn,
                                MonsterTrait::Poisonous, MonsterTrait::Weakening};
    auto monster = Monster{{Level{1}, 10_HP, 5_damage}, {}, std::move(traits)};
    it("should be applied", [&] {
      Hero hero;
      AssertThat(attack(hero, monster), Equals(Summary::Safe));
      {
        AssertThat(hero.has(HeroDebuff::Corroded), IsTrue());
        AssertThat(hero.getIntensity(HeroDebuff::Cursed), Equals(1u));
        AssertThat(hero.has(HeroDebuff::ManaBurned), IsTrue());
        AssertThat(hero.has(HeroDebuff::Poisoned), IsTrue());
        AssertThat(hero.getIntensity(HeroDebuff::Weakened), Equals(1u));
      }
    });
    it("should not be applied when hero takes no damage (except curse)", [&] {
      Hero hero;
      AssertThat(hero.receive(TaurogItem::Wereward), IsTrue());
      AssertThat(attack(hero, monster), Equals(Summary::Win));
      {
        AssertThat(hero.has(HeroDebuff::Corroded), IsFalse());
        AssertThat(hero.getIntensity(HeroDebuff::Cursed), Equals(1u));
        AssertThat(hero.has(HeroDebuff::ManaBurned), IsFalse());
        AssertThat(hero.has(HeroDebuff::Poisoned), IsFalse());
        AssertThat(hero.has(HeroDebuff::Weakened), IsFalse());
      }
    });
  });

  describe("Monster death gaze", [] {
    class CustomMonsterTraits : public MonsterTraits
    {
    public:
      static MonsterTraits withDeathGaze(unsigned char percent)
      {
        CustomMonsterTraits traits;
        traits.deathGaze_ = DeathGaze{percent};
        return static_cast<MonsterTraits>(traits);
      }
    };
    it("should petrify hero with low health (<50%)", [] {
      Hero hero;
      Monster gorgon(MonsterType::Gorgon, Level{1});
      AssertThat(attack(hero, gorgon), Equals(Summary::Win));
      hero.loseHitPointsOutsideOfFight(3, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(4));
      Monster gorgon2(MonsterType::Gorgon, Level{1});
      AssertThat(attack(hero, gorgon2), Equals(Summary::Petrified));
    });
    it("should be available with 100% intensity", [] {
      Hero hero;
      auto traits = CustomMonsterTraits::withDeathGaze(100);
      auto monster = Monster{{Level{1}, 100_HP, 1_damage}, {}, std::move(traits)};
      hero.add(HeroStatus::DeathProtection);
      AssertThat(attack(hero, monster), Equals(Summary::Safe));
      AssertThat(hero.has(HeroStatus::DeathProtection), IsTrue());
      hero.recover(100, noOtherMonsters);
      hero.loseHitPointsOutsideOfFight(1, noOtherMonsters);
      AssertThat(attack(hero, monster), Equals(Summary::Safe));
      AssertThat(hero.has(HeroStatus::DeathProtection), IsFalse());
      hero.recover(100, noOtherMonsters);
      hero.loseHitPointsOutsideOfFight(1, noOtherMonsters);
      AssertThat(attack(hero, monster), Equals(Summary::Petrified));
    });
    it("should be available with 101% intensity", [] {
      Hero hero;
      MonsterTraits traits = CustomMonsterTraits::withDeathGaze(101);
      auto monster = Monster{{Level{1}, 10_HP, 1_damage}, {}, std::move(traits)};
      AssertThat(attack(hero, monster), Equals(Summary::Petrified));
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
    Monster& burning = allMonsters.emplace_back(MonsterStats{Level{1}, 10_HP, 1_damage});
    Monster& nextTarget = allMonsters.emplace_back(MonsterType::MeatMan, Level{1});
    it("should occur on physical attack to other monster", [&] {
      AssertThat(Magic::cast(hero, burning, Spell::Burndayraz, allMonsters, resources), Equals(Summary::Safe));
      AssertThat(burning.getHitPoints(), Equals(6u));
      AssertThat(attack(hero, nextTarget, allMonsters, resources), Equals(Summary::Safe));
      AssertThat(burning.isBurning(), IsFalse());
      AssertThat(burning.getHitPoints(), Equals(5u));
    });
    it("should count as a burning kill", [&] {
      hero.recoverManaPoints(2);
      AssertThat(Magic::cast(hero, burning, Spell::Burndayraz, allMonsters, resources), Equals(Summary::Safe));
      AssertThat(burning.getHitPoints(), Equals(1u));
      hero.followDeity(God::GlowingGuardian, 0, resources);
      AssertThat(hero.getPiety(), Equals(5u));
      AssertThat(attack(hero, nextTarget, allMonsters, resources), Equals(Summary::Safe));
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
