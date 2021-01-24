#pragma once

#include "GameState.hpp"
#include "SolverTools.hpp"

#include <optional>

enum class Solver { GeneticAlgorithm, SimulatedAnnealing };

std::optional<Solution> run(Solver solver, GameState initialState);
