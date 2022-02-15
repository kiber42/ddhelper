#include "solver/GameState.hpp"
#include "solver/Solution.hpp"

namespace solver
{
  Step generateValidStep(const GameState& state);
  Step generateRandomValidStep(const GameState& state);
  std::vector<Step> generateAllValidSteps(const GameState& state);

  bool isValid(Step step, const GameState& state);

  GameState apply(const Step& step, GameState state);
  GameState apply(const Solution& solution, GameState state);

  void print(Solution solution, GameState state);
} // namespace solver
