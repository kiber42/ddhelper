#pragma once

#include "solver/GameState.hpp"
#include "solver/Solution.hpp"

#include <optional>

enum class Solver
{
  GeneticAlgorithm,
  TreeSearch,
};

std::optional<Solution> run(Solver solver, GameState initialState);
