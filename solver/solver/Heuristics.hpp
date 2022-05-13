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

  struct RegenFightResult
  {
    unsigned numAttacks{0};
    unsigned numSquaresUncovered{0};
    auto operator<=>(const RegenFightResult&) const = default;
  };

  /** @brief Evaluate whether a melee + recovery strategy can be used to defeat the monster.
   *  No simplifying assumptions are made.
   *  @returns Number of attacks and squares to uncover, or nullopt if the monster cannot be defeated this way.
   **/
  std::optional<RegenFightResult> checkRegenFight(Hero hero, Monster monster);

  /** @brief Evaluate whether a melee + recovery strategy can be used to defeat the monster.
   *  Makes a number of simplifications, e.g. death protection is ignored, and the traits stabber, determined, and
   *  berserking are not considered correctly.  Does not take any resources except black space into account.
   *  @returns Number of attacks and squares to uncover, or nullopt if the monster cannot be defeated this way.
   **/
  std::optional<RegenFightResult> checkRegenFightFast(const Hero& hero, const Monster& monster);
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState state);
