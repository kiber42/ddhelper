#pragma once

#include "ui/State.hpp"
#include "ui/Utils.hpp"

#include "solver/Solution.hpp"
#include "solver/Solver.hpp"

#include <optional>
#include <vector>

namespace ui
{
  class RunSolver
  {
  public:
    ActionResultUI operator()(const State& state);

  private:
    std::optional<std::vector<Step>> solverSteps;
    int solutionIndex{0};
    bool noSolutionFound{false};
    Solver selectedSolver{Solver::GeneticAlgorithm};
  };
} // namespace ui
