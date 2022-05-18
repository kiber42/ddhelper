#pragma once

#include "solver/GameState.hpp"
#include "solver/Solution.hpp"

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"

#include <optional>

namespace heuristics
{
  //! Return reference to highest-level monster from the list provided
  const Monster& strongest(const Monsters& monsters);

  //! Return pointer to monsters, stable-sorted by level, highest level first
  std::vector<const Monster*> sorted_by_level(const Monsters& monsters);

  enum class OneShotResult
  {
    None, // Hero safe, but monster not defeated
    Danger, // Hero dead or loses death protection
    VictoryFlawless,
    VictoryDamaged,
    VictoryDeathProtectionLost,
    VictoryGetindareOnly,
  };

  //! Check if the monster can be defeated with a single hit
  OneShotResult checkOneShot(const Hero& hero, const Monster& monster);

  /** Return true if there are enough one-shottable monsters to level up.
   *  Will not use death protection to achieve the level up.
   **/
  bool checkLevelCatapult(const Hero& hero, const Monsters& monsters);

  //! Return true if hero can defeat monster by attacking (repeatedly)
  bool checkMeleeOnly(Hero hero, Monster monster);

  struct RegenFightResult
  {
    unsigned numAttacks{0};
    unsigned numSquares{0};
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

  struct CatapultRegenFightResult
  {
    RegenFightResult beforeCatapult;
    RegenFightResult afterCatapult;
    unsigned numSquares() const { return beforeCatapult.numSquares + afterCatapult.numSquares; }
    unsigned numAttacks() const { return beforeCatapult.numAttacks + afterCatapult.numAttacks; }
    auto operator<=>(const CatapultRegenFightResult&) const = default;
  };

  /** @brief Evaluate if a monster can be defeated using regen fighting and a level catapult
   *  Note: The function assumes that a level catapult is available at any time without checking.
   *  @returns information on how the number of attacks and squares to uncover if the fight can be one, or nullopt.
   **/
  std::optional<CatapultRegenFightResult> checkRegenFightWithCatapult(Hero hero, Monster monster);
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState state);
