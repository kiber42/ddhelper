#include "solver/Scenario.hpp"

#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterStats.hpp"
#include "engine/MonsterTypes.hpp"
#include "engine/Spells.hpp"

DungeonSetup getSetupForScenario(Scenario scenario)
{
  if (scenario == Scenario::TheMonsterMachine1)
  {
    return {HeroClass::Monk,
            HeroRace::Human,
            BlacksmithItem::Sword,
            AlchemistSeal::CompressionSeal,
            BossReward::DragonShield,
            std::set{Potion::HealthPotion, Potion::ManaPotion, Potion::BurnSalve, Potion::CanOfWhupaz},
            {},
            MageModifier::ExtraHealthBoosters,
            BazaarModifier::Apothecary,
            {}};
  }
  assert(false);
  return DungeonSetup{};
}

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
      (void)hero.receive(Potion::ManaPotion);
    (void)hero.receiveFreeSpell(Spell::Burndayraz);
    (void)hero.receiveFreeSpell(Spell::Weytwut);
    return hero;
  }
  case Scenario::HalflingTrial:
  {
    Hero hero(HeroClass::Fighter, HeroRace::Halfling);
    hero.clearInventory();
    (void)hero.receive(Spell::Getindare);
    (void)hero.receive(Spell::Burndayraz);
    (void)hero.receive(Spell::Bysseps);
    (void)hero.receive(BlacksmithItem::Shield);
    hero.add(HeroStatus::Pessimist);
    return hero;
  }
  case Scenario::TheThirdAct:
  {
    Hero hero;
    SimpleResources ignore;
    hero.followDeity(God::TikkiTooki, 0, ignore);
    hero.getFaith().gainPiety(22);
    hero.addGold(100 - hero.gold());
    hero.clearInventory();
    (void)hero.receive(MiscItem::TikkisCharm);
    hero.add(HeroStatus::Pessimist);
    return hero;
  }
  case Scenario::TheMonsterMachine1:
  {
    auto accoLite = Hero{getSetupForScenario(scenario), {}};
    accoLite.setName("Acco Lite");
    return accoLite;
  }
  case Scenario::TrueGrit:
  {
    DungeonSetup setup;
    setup.startingEquipment.clear();
    setup.startingEquipment.insert(MiscItem::InfiniteManaPotions);
    auto hero = Hero{std::move(setup), {}};
    SimpleResources ignoreFreeSpell;
    hero.followDeity(God::BinlorIronshield, 1000u, ignoreFreeSpell);
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
    monsters.emplace_back(Monster{MonsterType::Goblin, Level{1}});
    monsters.emplace_back(Monster{"Djinn level 1", {Level{1}, 19_HP, 99_damage}, {}, {MonsterTrait::Retaliate}});
    monsters.emplace_back(
        Monster{"Zombie level 2", {Level{2}, 10_HP, 27_damage}, {}, {MonsterTrait::Undead, MonsterTrait::Bloodless}});
    monsters.emplace_back(Monster{
        "Eeblis (level 3)", {Level{3}, 28_HP, 999_damage}, {}, {MonsterTrait::Retaliate, MonsterTrait::FirstStrike}});
    break;
  case Scenario::HalflingTrial:
    monsters.emplace_back(Monster{"Goblin level 2", {Level{2}, 17_HP, 8_damage}, {}, {MonsterTrait::FirstStrike}});
    monsters.emplace_back(Monster{MonsterType::Warlock, Level{2}});
    monsters.emplace_back(Monster{"Meat Man level 2", {Level{2}, 30_HP, 4_damage}, {100_magicalresist}, {}});
    monsters.emplace_back(
        Monster{"Goblin level 2", {Level{2}, 15_HP, 8_damage}, {100_magicalresist}, {MonsterTrait::FirstStrike}});
    monsters.emplace_back(Monster{"Zombie level 3",
                                  {Level{3}, 39_HP, 12_damage},
                                  {100_magicalresist},
                                  {MonsterTrait::Bloodless, MonsterTrait::Undead}});
    monsters.emplace_back(Monster{"Jörmungandr (level 4)",
                                  {Level{4}, 58_HP, 21_damage},
                                  {},
                                  {MonsterTrait::FirstStrike, MonsterTrait::Poisonous}});
    break;
  case Scenario::TheThirdAct:
    for (int i = 0; i < 5; ++i)
      monsters.emplace_back(MonsterType::MeatMan, Level{1});
    monsters.emplace_back(Monster{"Puryton (level 1)", {Level{1}, 53_HP, 15_damage}, {}, {}});
    break;
  case Scenario::TheMonsterMachine1:
  {
    constexpr std::array types = {
        MonsterType::Bandit,  MonsterType::DragonSpawn, MonsterType::Goat,    MonsterType::Goblin, MonsterType::Golem,
        MonsterType::GooBlob, MonsterType::Gorgon,      MonsterType::MeatMan, MonsterType::Naga,   MonsterType::Serpent,
        MonsterType::Warlock, MonsterType::Wraith,      MonsterType::Zombie};
    auto typeIndex = std::uniform_int_distribution<size_t>{0u, types.size() - 1u};
    auto generator = std::mt19937{std::random_device{}()};
    auto level = Level{1};
    for (auto count : {10, 5, 4, 4, 4, 3, 3, 3, 2})
    {
      for (int i = 0; i < count; ++i)
      {
        auto monster = Monster{types[typeIndex(generator)], level, DungeonMultiplier{1.3f}};
        monster.makeCorrosive();
        monsters.emplace_back(std::move(monster));
      }
      level.increase();
    }
    monsters.emplace_back(Monster{
        "Chzar", MonsterStats{Level{10}, 1200_HP, 50_damage}, {}, {MonsterTrait::Retaliate, MonsterTrait::Corrosive}});
    // Level also has 44 Mysterious Murkshades (Corrosive!)
    break;
  }
  case Scenario::TrueGrit:
  {
    monsters.emplace_back(Monster{"Druid", MonsterStats{Level{3}, 8_HP, 5_damage}, {}, {}});
    monsters.emplace_back(Monster{
        "Catglove", MonsterStats{Level{4}, 75_HP, 4_damage}, {25_physicalresist}, {MonsterTrait::MagicalAttack}});
  }
  }
  return monsters;
}

SimpleResources getResourcesForScenario(Scenario scenario)
{
  if (scenario == Scenario::TheThirdAct)
  {
    auto resourceSet = ResourceSet{};
    resourceSet.shops = {Item{Potion::HealthPotion}, ShopItem::VenomDagger, ShopItem::BadgeOfHonour};
    resourceSet.altars = {God::TikkiTooki};
    auto resources = SimpleResources{std::move(resourceSet), 2};
    resources.revealTile(); // 2 x 2 - 1 = 3 hidden tiles
    return resources;
  }
  else if (scenario == Scenario::TheMonsterMachine1)
  {
    auto setup = getSetupForScenario(scenario);
    auto resources = SimpleResources{ResourceSet{setup}, 20};
    resources.ruleset = Ruleset::MonsterMachine1;
    // Ensure Wonafyt is present and is found early
    auto& spells = resources().spells;
    assert(spells.size() >= 2);
    if (auto spellIter = std::find(begin(spells), end(spells), Spell::Wonafyt); spellIter != end(spells))
    {
      // Swap Wonafyt to front
      *spellIter = spells.front();
    }
    else if (spells.front() == Spell::Burndayraz)
      // Overwrite first spell that is not Burndayraz
      spells[1] = Spell::Burndayraz;
    spells.front() = Spell::Wonafyt;
    return resources;
  }
  else if (scenario == Scenario::TrueGrit)
  {
    auto resourceSet = ResourceSet{};
    resourceSet.numWalls = 112;
    resourceSet.altars = {God::BinlorIronshield};
    return SimpleResources{std::move(resourceSet), 0 /* no hidden tiles */};
  }
  return SimpleResources{{}, 0 /* no hidden tiles */};
}
