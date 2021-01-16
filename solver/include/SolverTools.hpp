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

namespace solver
{
  Step generateRandomStep();
  Step generateRandomValidStep(const SolverState& state);
  bool isValid(Step step, const SolverState& state);

  SolverState apply(const Step& step, SolverState state);
  SolverState apply(const Solution& solution, SolverState state);

  void print(Solution solution, SolverState state);
} // namespace Solver