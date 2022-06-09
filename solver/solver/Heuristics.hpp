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

  /** Select suitable actions to level up hero using one-shot melee attacks
   *  @returns selected steps, or an empty solution if no catapult was found
   **/
  Solution buildLevelCatapult(GameState state);

  //! Return true if hero can defeat monster by attacking (repeatedly)
  bool checkMeleeOnly(Hero hero, Monster monster);

  struct RegenFightResult
  {
    unsigned numAttacks{0};
    unsigned numSquares{0};
    unsigned numAttacksBeforeCatapult{0};
    auto operator<=>(const RegenFightResult&) const = default;
  };

  /** @brief Evaluate whether a melee + recovery strategy can be used to defeat the monster.
   *  Makes a number of simplifications, e.g. death protection is ignored, and the traits stabber, determined, and
   *  berserking are not considered correctly.  Does not take any resources except black space into account.
   *  @returns Number of attacks and squares to uncover, or nullopt if the monster cannot be defeated this way.
   **/
  std::optional<RegenFightResult> checkRegenFightFast(const Hero& hero, const Monster& monster);

  /** @brief Evaluate whether a melee + recovery strategy can be used to defeat the monster.
   *  No simplifying assumptions are made.
   *  @returns Solution consisting of Attack and Uncover steps; empty solution if none found.
   **/
  Solution checkRegenFight(Hero hero, Monster monster);

  /** @brief Convert full solution into summary
   *  Currently only used for tests
   *  Counts the number of attack and uncover steps in solution to fill result
   **/
  RegenFightResult toRegenFightResult(const Solution& solution);

  /** @brief Evaluate if a monster can be defeated using regen fighting and a level catapult
   *  Note: The function assumes that a level catapult is available at any time without checking this.
   *  @returns Solution with a placeholder NoOp step where the level catapult should go; empty solution if none found.
   **/
  Solution checkRegenFightWithCatapult(Hero hero, Monster monster);
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState state);

constexpr const char* toString(heuristics::OneShotResult result)
{
  switch (result)
  {
  case heuristics::OneShotResult::None:
    return "None";
  case heuristics::OneShotResult::Danger:
    return "Danger";
  case heuristics::OneShotResult::VictoryFlawless:
    return "Flawless";
  case heuristics::OneShotResult::VictoryDamaged:
    return "Damaged";
  case heuristics::OneShotResult::VictoryDeathProtectionLost:
    return "Death Protection Lost";
  case heuristics::OneShotResult::VictoryGetindareOnly:
    return "Getindare Only";
  }
}

constexpr const char* toString(heuristics::CatapultResult result)
{
  switch (result)
  {
  case heuristics::CatapultResult::None:
    return "None";
  case heuristics::CatapultResult::Flawless:
    return "Flawless";
  case heuristics::CatapultResult::Damaged:
    return "Damaged";
  case heuristics::CatapultResult::DeathProtectionLost:
    return "Death Protection Lost";
  case heuristics::CatapultResult::DamagedAndDeathProtectionLost:
    return "Damaged and Death Protection Lost";
  }
}

inline std::string toString(const heuristics::RegenFightResult& result)
{
  if (result.numAttacksBeforeCatapult >= result.numAttacks)
    return std::to_string(result.numAttacks) + " attack(s) + " + std::to_string(result.numSquares) + " square(s)";
  return std::to_string(result.numAttacks) + " attack(s) + " + std::to_string(result.numSquares) +
         " square(s); level catapult after " + std::to_string(result.numAttacksBeforeCatapult) + " attack(s)";
}
