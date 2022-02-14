#include "solver/GameState.hpp"
#include "solver/Scenario.hpp"
#include "solver/Solution.hpp"
#include "solver/Solver.hpp"
#include "solver/SolverTools.hpp"

#include <iostream>

void testPrinting()
{
  Solution solution;
  solution.emplace_back(Attack{});
  solution.emplace_back(Uncover{5});
  solution.emplace_back(Buy{ShopItem::Spoon});
  solution.emplace_back(Find{Spell::Burndayraz});
  solution.emplace_back(Cast{Spell::Getindare});
  solution.emplace_back(Follow{God::Taurog});
  solution.emplace_back(Convert{Spell::Apheelsik});
  solution.emplace_back(Request{Boon::Humility});
  solution.emplace_back(Desecrate{God::BinlorIronshield});
  solution.emplace_back(Use{ShopItem::BattlemageRing});
  std::cout << toString(solution) << '\n';
}

void testSolve(Solver solver, Scenario scenario)
{
  GameState state{getHeroForScenario(scenario), getMonstersForScenario(scenario), {}, getResourcesForScenario(scenario)};

  auto solution = run(solver, state);
  if (solution)
  {
    std::cout << toString(*solution) << '\n';
    solver::print(*solution, state);
  }
  else
    std::cout << "No solution found.\n";
}

int main()
{
  testSolve(Solver::GeneticAlgorithm, Scenario::HalflingTrial);
  return 0;
}
