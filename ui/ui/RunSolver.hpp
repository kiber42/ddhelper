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
    /** Create solver UI window.
     *  The bool in the return value is set if the user requested an Undo.
     **/
    std::pair<ActionResultUI, bool> operator()(const State& state);

  private:
    std::optional<std::vector<Step>> solverSteps;
    size_t solutionIndex{0};
    bool noSolutionFound{false};
    Solver selectedSolver{Solver::GeneticAlgorithm};
  };
} // namespace ui
