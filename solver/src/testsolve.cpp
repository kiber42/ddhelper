#include "Solution.hpp"

#include "Scenario.hpp"
#include "Solver.hpp"

#include "MonsterTypes.hpp"

#include <iostream>

void testPrinting()
{
  Solution solution;
  solution.emplace_back(Attack{});
  solution.emplace_back(Uncover{5});
  solution.emplace_back(Buy{Item::Spoon});
  solution.emplace_back(Find{Spell::Burndayraz});
  solution.emplace_back(Cast{Spell::Getindare});
  solution.emplace_back(Follow{God::Taurog});
  solution.emplace_back(Convert{Spell::Apheelsik});
  solution.emplace_back(Request{Boon::Humility});
  solution.emplace_back(Desecrate{God::BinlorIronshield});
  solution.emplace_back(Use{Item::BattlemageRing});
  std::cout << toString(solution) << '\n';
}

void testSolve(Solver solver, Scenario scenario)
{
  SolverState state{getHeroForScenario(scenario), getMonstersForScenario(scenario), {}};

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
  //testSolve(Solver::SimulatedAnnealing, Scenario::HalflingTrial);
  testSolve(Solver::GeneticAlgorithm, Scenario::HalflingTrial);
  return 0;
}
