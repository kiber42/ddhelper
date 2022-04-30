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
      case heuristics::OneShotType::GetindareOnly:
        return "GetindareOnly";
      }
      return "[unsupported value]";
    }
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
      const Monster hitAndRun{MonsterStats{Level{1}, 1_HP, 100_damage}, {}, {}};
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
