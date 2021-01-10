#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Resources.hpp"
#include "Solution.hpp"

#include <optional>
#include <vector>

using MonsterPool = std::vector<Monster>;

struct SolverState
{
  Hero hero{};
  MonsterPool pool{};
  Resources resources{};
};

enum class Solver { GeneticAlgorithm, SimulatedAnnealing };

std::optional<Solution> run(Solver solver, SolverState initialState);

SolverState apply(Step step, SolverState state);
SolverState apply(Solution solution, SolverState state);
void print(Solution solution, SolverState state);

bool isValid(Step step, const SolverState& state);
