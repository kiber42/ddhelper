#include "solver/Scenario.hpp"

#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterStats.hpp"
#include "engine/MonsterTypes.hpp"
#include "engine/Spells.hpp"

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
      hero.receive(Potion::ManaPotion);
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
    hero.receive(BlacksmithItem::Shield);
    hero.add(HeroStatus::Pessimist);
    return hero;
  }
  case Scenario::TheThirdAct:
  {
    Hero hero;
    hero.followDeity(God::TikkiTooki, 0);
    hero.getFaith().gainPiety(22);
    hero.addGold(100 - hero.gold());
    hero.clearInventory();
    hero.receive(MiscItem::TikkisCharm);
    hero.add(HeroStatus::Pessimist);
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
    monsters.emplace_back(Monster{"Djinn level 1", {Level{1}, 19_HP, 99_damage}, {}, {MonsterTrait::Retaliate}});
    monsters.emplace_back(
        Monster{"Zombie level 2", {Level{2}, 10_HP, 27_damage}, {}, {MonsterTrait::Undead, MonsterTrait::Bloodless}});
    monsters.emplace_back(Monster{
        "Eeblis (level 3)", {Level{3}, 28_HP, 999_damage}, {}, {MonsterTrait::Retaliate, MonsterTrait::FirstStrike}});
    break;
  case Scenario::HalflingTrial:
    monsters.emplace_back(Monster{"Goblin level 2", {Level{2}, 17_HP, 8_damage}, {}, {MonsterTrait::FirstStrike}});
    monsters.emplace_back(Monster{MonsterType::Warlock, 2});
    monsters.emplace_back(Monster{"Meat Man level 2", {Level{2}, 30_HP, 4_damage}, {0, 100}, {}});
    monsters.emplace_back(
        Monster{"Goblin level 2", {Level{2}, 15_HP, 8_damage}, {0, 100}, {MonsterTrait::FirstStrike}});
    monsters.emplace_back(Monster{
        "Zombie level 3", {Level{3}, 39_HP, 12_damage}, {0, 100}, {MonsterTrait::Bloodless, MonsterTrait::Undead}});
    monsters.emplace_back(Monster{"Jörmungandr (level 4)",
                                  {Level{4}, 58_HP, 21_damage},
                                  {},
                                  {MonsterTrait::FirstStrike, MonsterTrait::Poisonous}});
    break;
  case Scenario::TheThirdAct:
    for (int i = 0; i < 5; ++i)
      monsters.emplace_back(MonsterType::MeatMan, 1);
    monsters.emplace_back(Monster{"Puryton (level 1)", {Level{1}, 53_HP, 15_damage}, {}, {}});
    break;
  }
  return monsters;
}

SimpleResources getResourcesForScenario(Scenario scenario)
{
  if (scenario == Scenario::TheThirdAct)
  {
    ResourceSet resourceSet;
    resourceSet.shops = {Item{Potion::HealthPotion}, ShopItem::VenomDagger, ShopItem::BadgeOfHonour};
    resourceSet.altars = {God::TikkiTooki};
    SimpleResources resources{std::move(resourceSet), 2};
    resources.revealTile(); // 2 x 2 - 1 = 3 hidden tiles
    return resources;
  }
  return SimpleResources{{}, 0 /* no hidden tiles */};
}
