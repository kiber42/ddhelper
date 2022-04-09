#include "solver/GameState.hpp"
#include "solver/Solution.hpp"

namespace solver
{
  Step generateValidStep(const GameState& state, bool allowTargetChange);
  Step generateRandomValidStep(const GameState& state, bool allowTargetChange);
  std::vector<Step> generateAllValidSteps(const GameState& state, bool allowTargetChange);

  bool isValid(Step step, const GameState& state);

  GameState apply(const Step& step, GameState state);
  GameState apply(const Solution& solution, GameState state);

  void print(const Solution& solution, GameState state);
} // namespace solver
