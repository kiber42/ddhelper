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
}

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
      case heuristics::OneShotType::Flawless:
        return "Flawless";
      case heuristics::OneShotType::Damaged:
        return "Damaged";
      case heuristics::OneShotType::DeathProtectionLost:
        return "DeathProtectionLost";
      case heuristics::OneShotType::GetindareOnly:
        return "GetindareOnly";
      }
      return "[unsupported value]";
    }
  };

  template <class T>
  struct Stringizer<std::optional<T>>
  {
    static std::string ToString(const std::optional<T>& optional)
    {
      if (optional)
        return std::to_string(optional.value());
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
  describe("Solver tools", [] {
    it("strongest monster shall be found correctly", [] {
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
    it("one shot availability shall be assessed correctly", [] {
      Hero hero;
      const auto weak = Monster{MonsterType::Generic, Level{1}};
      AssertThat(heuristics::checkOneShot(hero, weak), Equals(heuristics::OneShotType::None));
      const Monster hitAndRun{{Level{1}, 1_HP, 100_damage}, {}, {}};
      AssertThat(heuristics::checkOneShot(hero, hitAndRun), Equals(heuristics::OneShotType::None));
      AssertThat(hero.receive(Spell::Getindare), IsTrue());
      AssertThat(heuristics::checkOneShot(hero, hitAndRun), Equals(heuristics::OneShotType::GetindareOnly));
      hero.gainLevel(noOtherMonsters);
      AssertThat(heuristics::checkOneShot(hero, weak), Equals(heuristics::OneShotType::Flawless));
      auto goblin = Monster{MonsterType::Goblin, Level{1}};
      AssertThat(heuristics::checkOneShot(hero, goblin), Equals(heuristics::OneShotType::Damaged));
      goblin.slow();
      AssertThat(heuristics::checkOneShot(hero, goblin), Equals(heuristics::OneShotType::Flawless));
    });
    it("level catapult availability shall be assessed correctly", [] {
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
    });
    it("regen fighting shall be simulated correctly for simple cases (1)", [] {
      Hero guard;
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::GooBlob, Level{1}}), !Equals(std::nullopt));
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::GooBlob, Level{2}}), Equals(std::nullopt));
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::MeatMan, Level{2}}), Equals(std::nullopt));
      AssertThat(guard.receive(BlacksmithItem::Sword), IsTrue());
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::MeatMan, Level{2}}), Equals(std::nullopt));
      AssertThat(guard.receive(BlacksmithItem::Shield), IsTrue());
      AssertThat(heuristics::checkRegenFight(guard, {MonsterType::MeatMan, Level{2}}), Equals(1u));
    });
    it("regen fighting shall be simulated correctly for simple cases (2)", [] {
      Hero guard;
      const Monster hitAndRun{{Level{1}, 1_HP, 100_damage}, {}, {}};
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(std::nullopt));
      guard.setHitPointsMax(100);
      guard.healHitPoints(100);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(std::nullopt));
      guard.setHitPointsMax(101);
      guard.healHitPoints(101);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(0u));
      guard.setHitPointsMax(10);
      guard.gainLevel(noOtherMonsters);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(0u));
    });
    it("regen fighting heuristics shall handle basic status effects correctly", [] {
      Hero guard;
      Monster slowMonster{{Level{1}, 9_HP, 5_damage}, {}, {}};
      AssertThat(heuristics::checkRegenFight(guard, slowMonster), Equals(1u));
      slowMonster.slow();
      AssertThat(heuristics::checkRegenFight(guard, slowMonster), Equals(1u));
      {
        Monster burningMonster{{Level{1}, 7_HP, 5_damage}, {}, {}};
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster), Equals(1u));
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster), Equals(0u));
      }
      {
        Monster burningMonster{{Level{1}, 11_HP, 5_damage}, {}, {}};
        AssertThat(heuristics::checkRegenFight(guard, burningMonster), Equals(std::nullopt));
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster), Equals(std::nullopt));
        burningMonster.burn(2);
        AssertThat(heuristics::checkRegenFight(guard, burningMonster), Equals(1u));
      }
      {
        Monster poisonedMonster{{Level{1}, 10_HP, 9_damage}, {}, {}};
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster), Equals(std::nullopt));
        AssertThat(poisonedMonster.poison(8), IsTrue());
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster), Equals(std::nullopt));
        AssertThat(poisonedMonster.poison(1), IsTrue());
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster), Equals(9u));
        AssertThat(poisonedMonster.poison(10), IsTrue());
        AssertThat(heuristics::checkRegenFight(guard, poisonedMonster), Equals(9u));
      }
    });
    it("regen fighting heuristics shall permit losing death protection on the final strike", [] {
      Hero guard;
      const Monster hitAndRun{{Level{1}, 1_HP, 100_damage}, {}, {}};
      const Monster thirdHitKills{{Level{1}, 12_HP, 4_damage}, {}, {}};
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(std::nullopt));
      AssertThat(heuristics::checkRegenFight(guard, thirdHitKills), Equals(3u));
      guard.add(HeroStatus::DeathProtection);
      AssertThat(heuristics::checkRegenFight(guard, hitAndRun), Equals(0u));
      AssertThat(heuristics::checkRegenFight(guard, thirdHitKills), Equals(0u));
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
