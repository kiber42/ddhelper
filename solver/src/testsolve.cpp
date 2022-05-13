#include "bandit/bandit.h"

#include "solver/GameState.hpp"
#include "solver/Heuristics.hpp"
#include "solver/Scenario.hpp"
#include "solver/Solution.hpp"
#include "solver/Solver.hpp"
#include "solver/SolverTools.hpp"

#include <iostream>

using namespace bandit;
using namespace snowhouse;

using namespace heuristics;

namespace
{
  Monsters noOtherMonsters;
} // namespace

std::ostream& operator<<(std::ostream& out, const Monster& monster)
{
  const auto description = describe(monster);
  if (!description.empty())
  {
    if (description.size() > 1)
      std::copy(description.begin(), description.end() - 1, std::ostream_iterator<std::string>(out, ", "));
    out << description.back();
  }
  return out;
}

namespace snowhouse
{
  template <>
  struct Stringizer<heuristics::OneShotType>
  {
    static std::string ToString(const heuristics::OneShotType& oneShotType)
    {
      switch (oneShotType)
      {
      case heuristics::OneShotType::None:
        return "None";
      case heuristics::OneShotType::Danger:
        return "Danger";
      case heuristics::OneShotType::VictoryFlawless:
        return "Flawless";
      case heuristics::OneShotType::VictoryDamaged:
        return "Damaged";
      case heuristics::OneShotType::VictoryDeathProtectionLost:
        return "DeathProtectionLost";
      case heuristics::OneShotType::VictoryGetindareOnly:
        return "GetindareOnly";
      }
      return "[unsupported value]";
    }
  };

  template <>
  struct Stringizer<RegenFightResult>
  {
    static std::string ToString(const RegenFightResult& result)
    {
      return std::to_string(result.numAttacks) + " attack(s) + " + std::to_string(result.numSquaresUncovered) +
             " square(s)";
    }
  };

  template <>
  struct Stringizer<std::optional<RegenFightResult>>
  {
    static std::string ToString(const std::optional<RegenFightResult>& result)
    {
      if (result)
        return Stringizer<RegenFightResult>::ToString(*result);
      else
        return "nullopt";
    }
  };

  template <>
  struct Stringizer<std::nullopt_t>
  {
    static std::string ToString(const std::nullopt_t&) { return "nullopt"; }
  };
} // namespace snowhouse

void testGeneticSolver()
{
  const auto scenario = Scenario::HalflingTrial;
  GameState state{
      getHeroForScenario(scenario), getMonstersForScenario(scenario), {}, 0, getResourcesForScenario(scenario)};

  auto solution = run(Solver::GeneticAlgorithm, state);
  if (solution)
  {
    std::cout << toString(*solution) << '\n';
    solver::print(*solution, state);
  }
  else
    std::cout << "No solution found.\n";

  describe("Dummy", [] { it("should compile", [] { AssertThat(true, IsTrue()); }); });
}

void testHeuristics()
{
  describe("Generic solver tools", [] {
    it("shall find the strongest monster correctly", [] {
      Monsters monsters;
      monsters.emplace_back(MonsterType::Generic, Level{6});
      monsters.emplace_back(MonsterType::Generic, Level{9});
      monsters.emplace_back(MonsterType::Generic, Level{9});
      monsters.emplace_back(MonsterType::Generic, Level{8});
      AssertThat(heuristics::strongest(monsters), Equals(monsters[1]));
      const auto monsters_sorted = heuristics::sorted_by_level(monsters);
      AssertThat(monsters_sorted[0], Equals(&monsters[1]));
      AssertThat(monsters_sorted[1], Equals(&monsters[2]));
      AssertThat(monsters_sorted[2], Equals(&monsters[3]));
      AssertThat(monsters_sorted[3], Equals(&monsters[0]));
    });
    it("shall correctly assess one shot availability", [] {
      Hero hero;
      const auto weak = Monster{MonsterType::Generic, Level{1}};
      AssertThat(heuristics::checkOneShot(hero, weak), Equals(heuristics::OneShotType::None));
      const Monster hitAndRun{{Level{1}, 1_HP, 100_damage}, {}, {}};
      AssertThat(heuristics::checkOneShot(hero, hitAndRun), Equals(heuristics::OneShotType::Danger));
      AssertThat(hero.receive(Spell::Getindare), IsTrue());
      AssertThat(heuristics::checkOneShot(hero, hitAndRun), Equals(heuristics::OneShotType::VictoryGetindareOnly));
      hero.gainLevel(noOtherMonsters);
      AssertThat(heuristics::checkOneShot(hero, weak), Equals(heuristics::OneShotType::VictoryFlawless));
      auto goblin = Monster{MonsterType::Goblin, Level{1}};
      AssertThat(heuristics::checkOneShot(hero, goblin), Equals(heuristics::OneShotType::VictoryDamaged));
      goblin.slow();
      AssertThat(heuristics::checkOneShot(hero, goblin), Equals(heuristics::OneShotType::VictoryFlawless));
    });
    it("shall correctly assess level catapult availability", [] {
      auto hero = Hero{HeroClass::Assassin, HeroRace::Goblin};
      Monsters monsters;
      hero.gainLevel(monsters);
      for (int i = 0; i < 10; ++i)
        monsters.emplace_back(MonsterType::GooBlob, Level{1});
      AssertThat(heuristics::checkLevelCatapult(hero, monsters), IsTrue());
      monsters.pop_back();
      AssertThat(heuristics::checkLevelCatapult(hero, monsters), IsFalse());
      monsters.emplace_back(MonsterType::GooBlob, Level{2});
      AssertThat(heuristics::checkLevelCatapult(hero, monsters), IsFalse());
      monsters.emplace_back(MonsterStats{Level{2}, 5_HP, 1_damage});
      AssertThat(heuristics::checkLevelCatapult(hero, monsters), IsTrue());
      monsters.pop_back();
      monsters.emplace_back(MonsterStats{Level{10}, 1_HP, 100_damage});
      hero.add(HeroStatus::DeathProtection);
      AssertThat(heuristics::checkLevelCatapult(hero, monsters), IsFalse());
    });
    it("shall correctly assess the winner of a melee-only battle", [] {
      AssertThat(heuristics::checkMeleeOnly({HeroClass::Guard}, {MonsterType::Warlock, Level{2}}), IsFalse());

      Hero priest{HeroClass::Priest};
      AssertThat(heuristics::checkMeleeOnly(priest, {MonsterType::Zombie, Level{2}}), IsFalse());
      AssertThat(priest.receive(BlacksmithItem::Sword) && priest.receive(BlacksmithItem::Shield), IsTrue());
      AssertThat(heuristics::checkMeleeOnly(priest, {MonsterType::Zombie, Level{2}}), IsTrue());

      Hero berserker{HeroClass::Berserker};
      AssertThat(heuristics::checkMeleeOnly(berserker, {MonsterType::Wraith, Level{2}}), IsFalse());
      AssertThat(berserker.receive(ShopItem::Whurrgarbl), IsTrue());
      AssertThat(heuristics::checkMeleeOnly(berserker, {MonsterType::Wraith, Level{2}}), IsTrue());

      Hero rogue{HeroClass::Rogue};
      rogue.gainExperienceNoBonuses(250, noOtherMonsters);
      AssertThat(heuristics::checkMeleeOnly(rogue, {MonsterType::Serpent, Level{7}}), IsTrue());
      AssertThat(heuristics::checkMeleeOnly(rogue, {MonsterType::Serpent, Level{8}}), IsFalse());

      Hero assassin{HeroClass::Assassin};
      assassin.gainLevel(noOtherMonsters);
      assassin.gainLevel(noOtherMonsters);
      assassin.loseHitPointsOutsideOfFight(assassin.getHitPoints() - 1, noOtherMonsters);
      AssertThat(heuristics::checkMeleeOnly(assassin, {MonsterType::MeatMan, Level{2}}), IsTrue());
    });
  });
  describe("Regen fight prediction", [] {
    it("shall be correct for simple cases (1)", [] {
      Hero guard;
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::GooBlob, Level{1}}), !Equals(std::nullopt));
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::GooBlob, Level{2}}), Equals(std::nullopt));
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::MeatMan, Level{2}}), Equals(std::nullopt));
      AssertThat(guard.receive(BlacksmithItem::Sword), IsTrue());
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::MeatMan, Level{2}}), Equals(std::nullopt));
      AssertThat(guard.receive(BlacksmithItem::Shield), IsTrue());
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::MeatMan, Level{2}}),
                 Equals(RegenFightResult{.numAttacks = 5u, .numSquaresUncovered = 1u}));
    });
    it("shall be correct for simple cases (2)", [] {
      Hero guard;
      const Monster hitAndRun{{Level{1}, 1_HP, 100_damage}, {}, {}};
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(std::nullopt));
      guard.setHitPointsMax(100);
      guard.healHitPoints(100);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(std::nullopt));
      guard.setHitPointsMax(101);
      guard.healHitPoints(1);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun),
                 Equals(RegenFightResult{.numAttacks = 1u, .numSquaresUncovered = 0u}));
      guard.setHitPointsMax(10);
      guard.gainLevel(noOtherMonsters);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun),
                 Equals(RegenFightResult{.numAttacks = 1u, .numSquaresUncovered = 0u}));
    });
    it("shall handle basic status effects correctly", [] {
      Hero guard;
      Monster slowMonster{{Level{1}, 9_HP, 5_damage}, {}, {}};
      const auto expected = RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 1u};
      AssertThat(heuristics::checkRegenFight(guard, slowMonster), Equals(expected));
      slowMonster.slow();
      AssertThat(heuristics::checkRegenFight(guard, slowMonster), Equals(expected));
      {
        Monster burningMonster{{Level{1}, 7_HP, 5_damage}, {}, {}};
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster),
                   Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 1u}));
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster),
                   Equals(RegenFightResult{.numAttacks = 1u, .numSquaresUncovered = 0u}));
      }
      {
        Monster burningMonster{{Level{1}, 11_HP, 5_damage}, {}, {}};
        AssertThat(heuristics::checkRegenFight(guard, burningMonster), Equals(std::nullopt));
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster), Equals(std::nullopt));
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster),
                   Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 1u}));
      }
      {
        Monster poisonedMonster{{Level{1}, 10_HP, 9_damage}, {}, {}};
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster), Equals(std::nullopt));
        AssertThat(poisonedMonster.poison(8), IsTrue());
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster), Equals(std::nullopt));
        AssertThat(poisonedMonster.poison(1), IsTrue());
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster),
                   Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 9u}));
        AssertThat(poisonedMonster.poison(10), IsTrue());
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster),
                   Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 9u}));
      }
    });
    it("shall permit losing death protection on the final strike", [] {
      Hero guard;
      const Monster hitAndRun{{Level{1}, 1_HP, 100_damage}, {}, {}};
      const Monster thirdHitKills{{Level{1}, 12_HP, 4_damage}, {}, {}};
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(std::nullopt));
      AssertThat(heuristics::checkRegenFight(guard, thirdHitKills),
                 Equals(RegenFightResult{.numAttacks = 3u, .numSquaresUncovered = 3u}));
      guard.add(HeroStatus::DeathProtection);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun),
                 Equals(RegenFightResult{.numAttacks = 1u, .numSquaresUncovered = 0u}));
      AssertThat(heuristics::checkRegenFight(guard, thirdHitKills),
                 Equals(RegenFightResult{.numAttacks = 3u, .numSquaresUncovered = 0u}));
    });
    it("shall heal a wounded hero before attacking when needed", [] {
      Hero rogue{HeroClass::Rogue, HeroRace::Gnome};

      AssertThat(heuristics::checkRegenFight(rogue, {MonsterType::Wraith, Level{1}}),
                 Equals(RegenFightResult{.numAttacks = 1u, .numSquaresUncovered = 0u}));
      AssertThat(heuristics::checkRegenFight(rogue, {MonsterType::MeatMan, Level{1}}),
                 Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 0u}));

      rogue.loseHitPointsOutsideOfFight(4, noOtherMonsters);
      AssertThat(rogue.getHitPoints(), Equals(1u));
      AssertThat(heuristics::checkRegenFight(rogue, {MonsterType::Wraith, Level{1}}),
                 Equals(RegenFightResult{.numAttacks = 1u, .numSquaresUncovered = 0u}));
      AssertThat(heuristics::checkRegenFight(rogue, {MonsterType::MeatMan, Level{1}}),
                 Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 1u}));
      AssertThat(heuristics::checkRegenFight(rogue, {MonsterType::MeatMan, Level{2}}), Equals(std::nullopt));

      rogue.gainLevel(noOtherMonsters);
      auto meatMan = Monster{MonsterType::MeatMan, Level{3}};
      AssertThat(heuristics::checkRegenFight(rogue, meatMan),
                 Equals(RegenFightResult{.numAttacks = 8u, .numSquaresUncovered = 20u}));
      rogue.loseHitPointsOutsideOfFight(4, noOtherMonsters);
      AssertThat(heuristics::checkRegenFight(rogue, meatMan),
                 Equals(RegenFightResult{.numAttacks = 8u, .numSquaresUncovered = 22u}));
      meatMan.takeDamage(10, DamageType::Physical);
      AssertThat(heuristics::checkRegenFight(rogue, meatMan),
                 Equals(RegenFightResult{.numAttacks = 8u, .numSquaresUncovered = 22u}));
      meatMan.takeDamage(1, DamageType::Physical);
      AssertThat(heuristics::checkRegenFight(rogue, meatMan),
                 Equals(RegenFightResult{.numAttacks = 7u, .numSquaresUncovered = 19u}));

      meatMan.takeDamage(16, DamageType::Physical);
      rogue.recover(1, noOtherMonsters);
      AssertThat(meatMan.getHitPoints(), Equals(25u));
      AssertThat(rogue.getDamageOutputVersus(meatMan), Equals(14u));
      AssertThat(rogue.getHitPoints(), Equals(8u));
      AssertThat(meatMan.getDamage(), Equals(7u));

      AssertThat(heuristics::checkRegenFight(rogue, meatMan),
                 Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 0u}));
      rogue.loseHitPointsOutsideOfFight(2, noOtherMonsters);
      AssertThat(heuristics::checkRegenFight(rogue, meatMan),
                 Equals(RegenFightResult{.numAttacks = 2u, .numSquaresUncovered = 1u}));
      rogue.loseHitPointsOutsideOfFight(2, noOtherMonsters);
      AssertThat(heuristics::checkRegenFight(rogue, meatMan),
                 Equals(RegenFightResult{.numAttacks = 4u, .numSquaresUncovered = 9u}));
    });
  });
}

go_bandit([] {
  testHeuristics();
  // testGeneticSolver();
});

int main(int argc, char* argv[])
{
  return bandit::run(argc, argv);
}
