#pragma once

#include "SolverTools.hpp"

#include <optional>

enum class Solver { GeneticAlgorithm, SimulatedAnnealing };

std::optional<Solution> run(Solver solver, SolverState initialState);
