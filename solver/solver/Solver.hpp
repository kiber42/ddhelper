#pragma once

#include "solver/GameState.hpp"
#include "solver/SolverTools.hpp"

#include <optional>

enum class Solver
{
  GeneticAlgorithm
};

std::optional<Solution> run(Solver solver, GameState initialState);
