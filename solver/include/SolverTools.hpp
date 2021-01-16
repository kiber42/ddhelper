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

Step randomStep();
Step validRandomStep(const SolverState& state);
bool isValid(Step step, const SolverState& state);

SolverState apply(Step step, SolverState state);
SolverState apply(Solution solution, SolverState state);

void print(Solution solution, SolverState state);
