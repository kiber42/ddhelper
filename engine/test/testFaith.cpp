#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/Faith.hpp"
#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"
#include "engine/Spells.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
  SimpleResources resources{EmptyResources{}};
} // namespace

void testFaith()
{
  describe("Glowing Guardian", [] {
    describe("Likes", [] {
      it("shall award 2 piety for suffering poisoned or mana burned", [] {
        Hero hero;
        hero.followDeity(God::GlowingGuardian, 0);
        const int initialPiety = hero.getPiety();
        Monster manaBurnMonster("", {1, 10, 1, 0}, {}, MonsterTraitsBuilder().addManaBurn());
        AssertThat(Combat::attack(hero, manaBurnMonster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getPiety() - initialPiety, Equals(2));
        AssertThat(Combat::attack(hero, manaBurnMonster, noOtherMonsters), Equals(Summary::Win));
        AssertThat(hero.getPiety() - initialPiety, Equals(2));
        Monster poisonMonster("", {1, 10, 1, 0}, {}, MonsterTraitsBuilder().addPoisonous());
        AssertThat(Combat::attack(hero, poisonMonster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getPiety() - initialPiety, Equals(4));
        AssertThat(Combat::attack(hero, poisonMonster, noOtherMonsters), Equals(Summary::Win));
        AssertThat(hero.getPiety() - initialPiety, Equals(4));
      });
    });
  });
  describe("Tikki Tooki", [] {
    describe("Likes", [] {
      it("shall award 5 piety for killing an XP-valuable enemy of lower level", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        hero.followDeity(God::TikkiTooki, 0);
        Monster monster(MonsterType::MeatMan, 1);
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(Combat::attack(hero, monster, noOtherMonsters), Equals(Summary::Win));
        AssertThat(hero.getPiety(), Equals(5));
        hero.gainExperienceNoBonuses(7, noOtherMonsters);
        Monster monster2(2, 1, 1);
        AssertThat(Combat::attack(hero, monster2, noOtherMonsters), Equals(Summary::LevelUp));
        AssertThat(hero.getLevel(), Equals(3));
        AssertThat(hero.getPiety(), Equals(5));
      });
    });
    describe("Dislikes", [] {
      it("shall subtract 3 piety for taking more than one hit from an enemy", [] {
        Hero hero;
        hero.addStatus(HeroStatus::Learning, 4);
        hero.getFaith().gainPiety(5);
        hero.followDeity(God::TikkiTooki, 0);
        Monsters meatMen{{MonsterType::MeatMan, 1}, {MonsterType::MeatMan, 1}};
        AssertThat(Combat::attack(hero, meatMen[0], meatMen), Equals(Summary::Safe));
        AssertThat(Combat::attack(hero, meatMen[0], meatMen), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(2));
        AssertThat(Combat::attack(hero, meatMen[0], meatMen), Equals(Summary::LevelUp));
        AssertThat(hero.getPiety(), Equals(0));
        AssertThat(meatMen[1].hasFirstStrike(), IsTrue());
        AssertThat(meatMen[1].isWeakening(), IsTrue());

        AssertThat(Combat::attack(hero, meatMen[1], meatMen), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(0));
        AssertThat(Combat::attack(hero, meatMen[1], meatMen), Equals(Summary::Win));
        AssertThat(hero.getPiety(), Equals(5));
      });
      it("shall accept tribute", [] {
        Hero hero;
        hero.followDeity(God::TikkiTooki, 0);
        hero.addGold(150 - hero.gold());
        AssertThat(hero.getPiety(), Equals(0));
        AssertThat(hero.gold(), Equals(150));
        for (int i = 1; i <= 10; ++i)
        {
          hero.request(Boon::Tribute, noOtherMonsters, resources);
          AssertThat(hero.getPiety(), Equals(i * 10));
          AssertThat(hero.gold(), Equals(150 - i * 15));
        }
      });
      it("shall ignore hits that do not cause damage", [] {
        Hero hero;
        hero.followDeity(God::TikkiTooki, 0);
        hero.request(Boon::Tribute, noOtherMonsters, resources);
        AssertThat(hero.getPiety(), Equals(10));
        Monster meatMan(1, 100, 3);
        AssertThat(Combat::attack(hero, meatMan, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(10));
        AssertThat(Combat::attack(hero, meatMan, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(7));
        hero.addStatus(HeroStatus::DamageReduction, 2);
        AssertThat(Combat::attack(hero, meatMan, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(4));
        hero.addStatus(HeroStatus::DamageReduction, 1);
        AssertThat(Combat::attack(hero, meatMan, noOtherMonsters), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(4));
      });
    });
  });
}
