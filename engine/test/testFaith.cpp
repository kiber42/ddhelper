#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/DungeonSetup.hpp"
#include "engine/Faith.hpp"
#include "engine/Hero.hpp"
#include "engine/Items.hpp"
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

  void cast(Hero& hero, Spell spell) { Magic::cast(hero, spell, noOtherMonsters, resources); }

  auto cast(Hero& hero, Monster& monster, Spell spell)
  {
    return Magic::cast(hero, monster, spell, noOtherMonsters, resources);
  }
} // namespace

void testFaith()
{
  describe("Glowing Guardian", [] {
    describe("[Likes]", [] {
      it("shall award 2 piety for suffering poisoned or mana burned", [] {
        Hero hero;
        hero.followDeity(God::GlowingGuardian, 0);
        const auto initialPiety = hero.getPiety();
        Monster manaBurnMonster("", {Level{1}, 10_HP, 1_damage}, {}, {MonsterTrait::ManaBurn});
        AssertThat(attack(hero, manaBurnMonster), Equals(Summary::Safe));
        AssertThat(hero.getPiety() - initialPiety, Equals(2u));
        AssertThat(attack(hero, manaBurnMonster), Equals(Summary::Win));
        AssertThat(hero.getPiety() - initialPiety, Equals(2u));
        Monster poisonMonster("", {Level{1}, 10_HP, 1_damage}, {}, {MonsterTrait::Poisonous});
        AssertThat(attack(hero, poisonMonster), Equals(Summary::Safe));
        AssertThat(hero.getPiety() - initialPiety, Equals(4u));
        AssertThat(attack(hero, poisonMonster), Equals(Summary::Win));
        AssertThat(hero.getPiety() - initialPiety, Equals(4u));
      });
    });
  });
  describe("Mystera Annur", [] {
    it("shall award piety for spending mana (1 piety per 2 MP)", [] {
      Hero hero;
      hero.setManaPointsMax(14);
      hero.recoverManaPoints(4);
      hero.followDeity(God::MysteraAnnur, 0);
      Monster meatMan(MonsterType::MeatMan, 1);
      AssertThat(cast(hero, meatMan, Spell::Burndayraz), Equals(Summary::Safe));
      AssertThat(hero.getPiety(), Equals(3u));
      cast(hero, Spell::Getindare);
      AssertThat(hero.getManaPoints(), Equals(5u));
      AssertThat(hero.getPiety(), Equals(4u));
      cast(hero, meatMan, Spell::Apheelsik);
      AssertThat(hero.getPiety(), Equals(7u));
      AssertThat(hero.getManaPoints(), Equals(0u));
    });
    it("shall award only 2 piety per 5 MP (1 after 3 MP, 1 after 2 MP) if her altar was prepared", [] {
      DungeonSetup setup;
      setup.altar = God::MysteraAnnur;

      Hero hero{setup, {}};
      hero.followDeity(God::MysteraAnnur, 0);
      cast(hero, Spell::Lemmisi);
      AssertThat(hero.getPiety(), Equals(0u));
      cast(hero, Spell::Lemmisi);
      AssertThat(hero.getPiety(), Equals(1u));
      cast(hero, Spell::Lemmisi);
      AssertThat(hero.getPiety(), Equals(2u));
      cast(hero, Spell::Lemmisi);
      AssertThat(hero.getPiety(), Equals(3u));
      cast(hero, Spell::Lemmisi);
      AssertThat(hero.getPiety(), Equals(4u));
      cast(hero, Spell::Lemmisi);
      AssertThat(hero.getPiety(), Equals(4u));

      setup.heroClass = HeroClass::Wizard;
      Hero wizard{setup, {}};
      wizard.followDeity(God::MysteraAnnur, 0);
      cast(wizard, Spell::Lemmisi);
      AssertThat(wizard.getPiety(), Equals(0u));
      cast(wizard, Spell::Lemmisi);
      AssertThat(wizard.getPiety(), Equals(0u));
      cast(wizard, Spell::Lemmisi);
      AssertThat(wizard.getPiety(), Equals(1u));
      cast(wizard, Spell::Lemmisi);
      AssertThat(wizard.getPiety(), Equals(1u));
      cast(wizard, Spell::Lemmisi);
      AssertThat(wizard.getPiety(), Equals(2u));
    });
  });
  describe("Tikki Tooki", [] {
    describe("[Likes]", [] {
      it("shall award 5 piety for killing an XP-valuable enemy of lower level", [] {
        Hero hero;
        hero.gainLevel(noOtherMonsters);
        hero.followDeity(God::TikkiTooki, 0);
        Monster monster(MonsterType::MeatMan, 1);
        AssertThat(attack(hero, monster), Equals(Summary::Safe));
        AssertThat(attack(hero, monster), Equals(Summary::Win));
        AssertThat(hero.getPiety(), Equals(5u));
        hero.gainExperienceNoBonuses(7, noOtherMonsters);
        Monster monster2(2, 1, 1);
        AssertThat(attack(hero, monster2), Equals(Summary::LevelUp));
        AssertThat(hero.getLevel(), Equals(3u));
        AssertThat(hero.getPiety(), Equals(5u));
      });
    });
    describe("[Dislikes]", [] {
      it("shall subtract 3 piety for taking more than one hit from an enemy", [] {
        Hero hero;
        hero.add(HeroStatus::Learning, 4);
        hero.getFaith().gainPiety(5);
        hero.followDeity(God::TikkiTooki, 0);
        Monsters meatMen{{MonsterType::MeatMan, 1}, {MonsterType::MeatMan, 1}};
        AssertThat(attack(hero, meatMen[0], meatMen, resources), Equals(Summary::Safe));
        AssertThat(attack(hero, meatMen[0], meatMen, resources), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(2u));
        AssertThat(attack(hero, meatMen[0], meatMen, resources), Equals(Summary::LevelUp));
        AssertThat(hero.getPiety(), Equals(0u));
        AssertThat(meatMen[1].has(MonsterTrait::FirstStrike), IsTrue());
        AssertThat(meatMen[1].has(MonsterTrait::Weakening), IsTrue());

        AssertThat(attack(hero, meatMen[1], meatMen, resources), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(0u));
        AssertThat(attack(hero, meatMen[1], meatMen, resources), Equals(Summary::Win));
        AssertThat(hero.getPiety(), Equals(5u));
      });
      it("shall subtract 3 piety for taking any hit if his altar was prepared", [] {
        DungeonSetup setup;
        setup.altar = God::TikkiTooki;
        Hero hero{setup, {}};
        hero.getFaith().gainPiety(10);
        hero.followDeity(God::TikkiTooki, 0);
        Monsters monsters{{MonsterType::MeatMan, 1}, {MonsterType::Wraith, 1}};
        Monster& meatMan = monsters[0];
        Monster& wraith = monsters[1];
        AssertThat(attack(hero, meatMan, monsters, resources), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(7u));
        AssertThat(attack(hero, meatMan, monsters, resources), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(4u));
        AssertThat(attack(hero, wraith, monsters, resources), Equals(Summary::Win));
        AssertThat(hero.getPiety(), Equals(1u));
      });
      it("shall accept tribute", [] {
        Hero hero;
        hero.followDeity(God::TikkiTooki, 0);
        hero.addGold(150 - hero.gold());
        AssertThat(hero.getPiety(), Equals(0u));
        AssertThat(hero.gold(), Equals(150u));
        for (unsigned i = 1; i <= 10; ++i)
        {
          hero.request(Boon::Tribute, noOtherMonsters, resources);
          AssertThat(hero.getPiety(), Equals(i * 10u));
          AssertThat(hero.gold(), Equals(150u - i * 15u));
        }
      });
      it("shall ignore hits that do not cause damage", [] {
        Hero hero;
        hero.followDeity(God::TikkiTooki, 0);
        hero.request(Boon::Tribute, noOtherMonsters, resources);
        AssertThat(hero.getPiety(), Equals(10u));
        Monster meatMan(1, 100, 3);
        AssertThat(attack(hero, meatMan), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(10u));
        AssertThat(attack(hero, meatMan), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(7u));
        hero.add(HeroStatus::DamageReduction, 2);
        AssertThat(attack(hero, meatMan), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(4u));
        hero.add(HeroStatus::DamageReduction, 1);
        AssertThat(attack(hero, meatMan), Equals(Summary::Safe));
        AssertThat(hero.getPiety(), Equals(4u));
      });
    });
  });
}
