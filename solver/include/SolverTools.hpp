#include "GameState.hpp"
#include "Solution.hpp"

namespace solver
{
  Step generateRandomStep();
  Step generateRandomValidStep(const GameState& state);
  bool isValid(Step step, const GameState& state);

  GameState apply(const Step& step, GameState state);
  GameState apply(const Solution& solution, GameState state);

  void print(Solution solution, GameState state);
} // namespace solver
