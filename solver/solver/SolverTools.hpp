#include "solver/GameState.hpp"
#include "solver/Solution.hpp"

namespace solver
{
  Step generateValidStep(const GameState& state, bool allowTargetChange);
  Step generateRandomValidStep(const GameState& state, bool allowTargetChange);
  std::vector<Step> generateAllValidSteps(const GameState& state, bool allowTargetChange);

  bool isValid(const Step& step, const GameState& state);

  void apply(const Step& step, GameState& state);
  void apply(const Solution& solution, GameState& state);

  void print(const Solution& solution, GameState state);
} // namespace solver
