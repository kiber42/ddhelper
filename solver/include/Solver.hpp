#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Solution.hpp"

#include <optional>
#include <vector>

using MonsterPool = std::vector<Monster>;

class Solver
{
public:
  Solver(const Hero&, const MonsterPool&, const Resources&);
  std::optional<Solution> run();
private:

};