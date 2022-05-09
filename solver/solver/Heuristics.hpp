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
    DeathProtectionLost,
    GetindareOnly,
    Danger,
  };

  const Monster& strongest(const Monsters& monsters);

  std::vector<const Monster*> sorted_by_level(const Monsters& monsters);

  OneShotType checkOneShot(const Hero& hero, const Monster& monster);

  bool checkLevelCatapult(const Hero& hero, const Monsters& monsters);

  /** @brief Evaluate whether a melee + recovery strategy can be used to defeat the monster.
   *  No simplifying assumptions are made.
   *  @returns Number of squares to uncover, or nullopt if the monster cannot be defeated this way.
   **/
  std::optional<unsigned> checkRegenFight(Hero hero, Monster monster);

  /** @brief Evaluate whether a melee + recovery strategy can be used to defeat the monster.
   *  Makes a number of simplifications, e.g. death protection is ignored, and the traits stabber and defiant are not
   *  considered correctly.  Does not take any resources except black space into account.
   *  @returns Number of squares to uncover, or nullopt if the monster cannot be defeated this way.
   **/
  std::optional<unsigned> checkRegenFightFast(const Hero& hero, const Monster& monster);
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState state);
