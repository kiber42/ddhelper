#include "Scenario.hpp"

#include "Hero.hpp"
#include "Items.hpp"
#include "MonsterStats.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

Hero getHeroForScenario(Scenario scenario)
{
  switch (scenario)
  {
  case Scenario::AgbaarsAcademySlowingPart2:
  {
    Hero hero;
    hero.gainLevel();
    hero.gainExperienceNoBonuses(4);
    hero.loseHitPointsOutsideOfFight(3);
    hero.clearInventory();
    for (int i = 0; i < 8; ++i)
      hero.receive(Item::ManaPotion);
    hero.receiveFreeSpell(Spell::Burndayraz);
    hero.receiveFreeSpell(Spell::Weytwut);
    return hero;
  }
  case Scenario::HalflingTrial:
  {
    Hero hero(HeroClass::Fighter, HeroRace::Halfling);
    hero.clearInventory();
    hero.receive(Spell::Getindare);
    hero.receive(Spell::Burndayraz);
    hero.receive(Spell::Bysseps);
    hero.receive(Item::Shield);
    hero.addStatus(HeroStatus::Pessimist);
    return hero;
  }
  }
}

std::vector<Monster> getMonstersForScenario(Scenario scenario)
{
  std::vector<Monster> pool;
  switch (scenario)
  {
  case Scenario::AgbaarsAcademySlowingPart2:
  {
    pool.emplace_back(Monster{MonsterType::Goblin, 1});
    pool.emplace_back(Monster{"Djinn level 1", {1, 19, 99, 0}, {}, MonsterTraitsBuilder().addRetaliate().get()});
    pool.emplace_back(
        Monster{"Zombie level 2", {2, 10, 27, 0}, {}, MonsterTraitsBuilder().addUndead().addBloodless().get()});
    pool.emplace_back(
        Monster{"Eeblis (level 3)", {3, 28, 999, 0}, {}, MonsterTraitsBuilder().addFirstStrike().addRetaliate().get()});
  }
  case Scenario::HalflingTrial:
  {
    pool.emplace_back(Monster{"Goblin level 2", {2, 17, 8, 0}, {}, MonsterTraitsBuilder().addFirstStrike()});
    pool.emplace_back(Monster{MonsterType::Warlock, 2});
    pool.emplace_back(Monster{"Meat Man level 2", {2, 30, 4, 0}, {0, 100}, {}});
    pool.emplace_back(Monster{"Goblin level 2", {2, 15, 8, 0}, {0, 100}, MonsterTraitsBuilder().addFirstStrike()});
    pool.emplace_back(
        Monster{"Zombie level 3", {3, 39, 12, 0}, {0, 100}, MonsterTraitsBuilder().addBloodless().addUndead()});
    pool.emplace_back(
        Monster{"Jörmungandr (level 4)", {4, 58, 21, 0}, {}, MonsterTraitsBuilder().addFirstStrike().addPoisonous()});
  }
  break;
  }
  return pool;
}
