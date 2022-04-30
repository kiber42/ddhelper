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
    });
    it("one shot availability shall be assessed correctly", [] {
      /* TODO
      Hero hero;
      Monster monster;
      */
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
