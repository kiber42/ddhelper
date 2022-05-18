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
    None,   // Hero safe, but monster not defeated
    Danger, // Hero dead or loses death protection
    VictoryFlawless,
    VictoryDamaged,
    VictoryDeathProtectionLost,
    VictoryGetindareOnly,
  };

  //! Check if the monster can be defeated with a single hit
  OneShotResult checkOneShot(const Hero& hero, const Monster& monster);

  enum class CatapultResult
  {
    None,                          // no "simple" way to achieve a level up
    Flawless,                      // there are sufficient one-shottable monsters to level up
    Damaged,                       // hero takes damage on final attack before levelling
    DeathProtectionLost,           // hero loses death protection on final attack
    DamagedAndDeathProtectionLost, // hero takes damage on semi-final attack and loses death protection on final attack
  };

  /** Evaluate whether there are enough one-shottable monsters to level up.
   *  Will largely ignore monsters that cannot be killed flawlessly, except in the final attack.
   *  If the hero has death protection, considers sacrificing it in the final attack and permits taking damage in the
   *  attack before that.
   *  @returns appropriate enum value
   **/
  CatapultResult checkLevelCatapult(const Hero& hero, const Monsters& monsters);

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
