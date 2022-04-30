#pragma once

#include "solver/GameState.hpp"
#include "solver/Solution.hpp"

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"

#include <optional>

namespace heuristics
{
  enum class OneShotType
  {
    None,
    Flawless,
    Damaged,
    GetindareOnly,
  };

  const Monster& strongest(const Monsters& monsters);

  std::vector<const Monster*> sorted_by_level(const Monsters& monsters);

  OneShotType checkOneShot(const Hero& hero, const Monster& monster);

  bool checkLevelCatapult(const Hero& hero, const Monsters& monsters);
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState state);
