#include "Scenario.hpp"

#include "Hero.hpp"
#include "Items.hpp"
#include "Monster.hpp"
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
    Monsters ignore;
    hero.gainLevel(ignore);
    hero.gainExperienceNoBonuses(4, ignore);
    hero.loseHitPointsOutsideOfFight(3, ignore);
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
  case Scenario::TheThirdAct:
  {
    Hero hero;
    hero.followDeity(God::TikkiTooki);
    hero.getFaith().gainPiety(22);
    hero.addGold(100 - hero.gold());
    hero.clearInventory();
    hero.receive(Item::TikkisCharm);
    return hero;
  }
  }
}

std::vector<Monster> getMonstersForScenario(Scenario scenario)
{
  Monsters monsters;
  switch (scenario)
  {
  case Scenario::AgbaarsAcademySlowingPart2:
    monsters.emplace_back(Monster{MonsterType::Goblin, 1});
    monsters.emplace_back(Monster{"Djinn level 1", {1, 19, 99, 0}, {}, MonsterTraitsBuilder().addRetaliate().get()});
    monsters.emplace_back(
        Monster{"Zombie level 2", {2, 10, 27, 0}, {}, MonsterTraitsBuilder().addUndead().addBloodless().get()});
    monsters.emplace_back(
        Monster{"Eeblis (level 3)", {3, 28, 999, 0}, {}, MonsterTraitsBuilder().addFirstStrike().addRetaliate().get()});
    break;
  case Scenario::HalflingTrial:
    monsters.emplace_back(Monster{"Goblin level 2", {2, 17, 8, 0}, {}, MonsterTraitsBuilder().addFirstStrike()});
    monsters.emplace_back(Monster{MonsterType::Warlock, 2});
    monsters.emplace_back(Monster{"Meat Man level 2", {2, 30, 4, 0}, {0, 100}, {}});
    monsters.emplace_back(Monster{"Goblin level 2", {2, 15, 8, 0}, {0, 100}, MonsterTraitsBuilder().addFirstStrike()});
    monsters.emplace_back(
        Monster{"Zombie level 3", {3, 39, 12, 0}, {0, 100}, MonsterTraitsBuilder().addBloodless().addUndead()});
    monsters.emplace_back(
        Monster{"Jörmungandr (level 4)", {4, 58, 21, 0}, {}, MonsterTraitsBuilder().addFirstStrike().addPoisonous()});
    break;
  case Scenario::TheThirdAct:
    for (int i = 0; i < 5; ++i)
      monsters.emplace_back(MonsterType::MeatMan, 1);
    monsters.emplace_back(Monster{"Puryton", {1, 53, 15, 0}, {}, {}});
    break;
  }
  return monsters;
}

SimpleResources getResourcesForScenario(Scenario scenario)
{
  ResourceSet resources{EmptyResources{}};
  if (scenario == Scenario::TheThirdAct)
  {
    resources.shops = {Item::HealthPotion, Item::VenomDagger, Item::BadgeOfHonour};
    resources.altars = {God::TikkiTooki};
    resources.numWalls = 3;
  }
  return SimpleResources{std::move(resources)};
}
