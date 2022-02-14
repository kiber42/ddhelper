#include "solver/Solver.hpp"
#include "solver/Fitness.hpp"

std::optional<Solution> runGeneticAlgorithm(GameState state);

std::optional<Solution> run(Solver solver, GameState initialState)
{
  switch (solver)
  {
  case Solver::GeneticAlgorithm:
    return runGeneticAlgorithm(std::move(initialState));
  }
}
