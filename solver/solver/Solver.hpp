#pragma once

#include "solver/GameState.hpp"
#include "solver/Solution.hpp"

#include <optional>

enum class Solver
{
  GeneticAlgorithm,
  TreeSearch,
  Heuristics,
  Last = Heuristics
};

std::optional<Solution> run(Solver solver, GameState initialState);

constexpr const char* toString(Solver solver)
{
  switch (solver)
  {
  case Solver::GeneticAlgorithm:
    return "Genetic Algorithm";
  case Solver::TreeSearch:
    return "Tree Search";
  case Solver::Heuristics:
    return "Heuristics";
  }
}
