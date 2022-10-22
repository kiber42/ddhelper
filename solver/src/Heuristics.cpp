#include "solver/Heuristics.hpp"

#include "solver/SolverTools.hpp"

#include "engine/Combat.hpp"
#include "engine/Magic.hpp"

#include <iostream>
#include <numeric>

namespace heuristics
{
  const Monster& strongest(const Monsters& monsters)
  {
    assert(!monsters.empty());
    return *std::max_element(begin(monsters), end(monsters),
                             [](auto& left, auto& right) { return left.getLevel() < right.getLevel(); });
  }

  std::vector<const Monster*> sorted_by_level(const Monsters& monsters)
  {
    std::vector<const Monster*> sorted(monsters.size());
    std::transform(begin(monsters), end(monsters), begin(sorted), [](const auto& monster) { return &monster; });
    std::stable_sort(begin(sorted), end(sorted),
                     [](auto left, auto right) { return left->getLevel() > right->getLevel(); });
    return sorted;
  }

  std::vector<size_t> sorted_by_level_index(const Monsters& monsters)
  {
    std::vector<size_t> sorted(monsters.size());
    std::iota(begin(sorted), end(sorted), 0u);
    std::stable_sort(begin(sorted), end(sorted), [&monsters](auto left, auto right) {
      return monsters[left].getLevel() > monsters[right].getLevel();
    });
    return sorted;
  }

  inline bool isSafeToAttack(const Hero& hero, const Monster& monster)
  {
    const bool willPetrify = !monster.isSlowed() && !hero.has(HeroStatus::DeathGazeImmune) &&
                             (monster.getDeathGazePercent() * hero.getHitPointsMax() > hero.getHitPoints() * 100u);
    const bool deadly =
        (willPetrify || hero.predictDamageTaken(monster.getDamage(), monster.damageType()) >= hero.getHitPoints());
    return !deadly;
  }

  OneShotResult checkOneShot(const Hero& hero, const Monster& monster)
  {
    const auto monsterDamageTaken = monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType());
    const bool willDefeatMonster = monsterDamageTaken >= monster.getHitPoints();
    if (willDefeatMonster)
    {
      if (hero.hasInitiativeVersus(monster))
        return OneShotResult::VictoryFlawless;
      if (isSafeToAttack(hero, monster))
        return OneShotResult::VictoryDamaged;
      if (hero.has(HeroStatus::DeathProtection))
        return OneShotResult::VictoryDeathProtectionLost;
      const bool firstStrikeHero =
          hero.has(HeroStatus::FirstStrikePermanent) || hero.has(HeroStatus::FirstStrikeTemporary);
      const bool canCast = !firstStrikeHero && hero.has(Spell::Getindare) &&
                           hero.getManaPoints() >= Magic::spellCosts(Spell::Getindare, hero);
      const bool firstStrikeMonster = monster.has(MonsterTrait::FirstStrike) && !monster.isSlowed();
      if (canCast && !firstStrikeMonster)
        return OneShotResult::VictoryGetindareOnly;
    }
    if (!isSafeToAttack(hero, monster))
      return OneShotResult::Danger;
    return OneShotResult::None;
  }

  CatapultResult checkLevelCatapult(const Hero& hero, const Monsters& monsters)
  {
    auto totalXp = 0_xp;
    auto xpTakingDamage = 0_xp;
    auto xpBarelyWin = 0_xp;
    for (const auto& monster : monsters)
    {
      if (!monster.grantsXP() || monster.isDefeated())
        continue;
      const auto oneShot = checkOneShot(hero, monster);
      if (oneShot == OneShotResult::None || oneShot == OneShotResult::Danger)
        continue;
      const auto xpGain = ExperiencePoints{hero.predictExperienceForKill(monster.getLevel(), monster.isSlowed())};
      if (oneShot == OneShotResult::VictoryFlawless)
        totalXp += xpGain;
      else if (oneShot == OneShotResult::VictoryDamaged)
        xpTakingDamage = std::max(xpTakingDamage, xpGain);
      else if (oneShot == OneShotResult::VictoryDeathProtectionLost || oneShot == OneShotResult::VictoryGetindareOnly)
        xpBarelyWin = std::max(xpBarelyWin, xpGain);
    }
    const auto xpNeeded = ExperiencePoints{hero.getXPforNextLevel() - hero.getXP()};
    if (totalXp >= xpNeeded)
      return CatapultResult::Flawless;
    if (totalXp + xpTakingDamage >= xpNeeded)
      return CatapultResult::Damaged;
    if (totalXp + xpTakingDamage + xpBarelyWin >= xpNeeded && hero.has(HeroStatus::DeathProtection))
    {
      if (totalXp + xpBarelyWin >= xpNeeded)
        return CatapultResult::DeathProtectionLost;
      return CatapultResult::DamagedAndDeathProtectionLost;
    }
    return CatapultResult::None;
  }

  namespace
  {
    thread_local Monsters ignoreMonsters;
    thread_local SimpleResources ignoreResources;

    void print_description(const std::vector<std::string>& description)
    {
      std::string s = std::accumulate(begin(description), end(description), std::string{},
                                      [](std::string a, std::string b) { return a + ", " + b; });
      if (!s.empty())
        std::cout << s.substr(2) << '\n';
    }

    void assertSafeToAttack(Hero hero, Monster monster)
    {
      const auto summary = Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
      if (summary == Summary::Death || summary == Summary::Petrified || summary == Summary::NotPossible)
      {
        std::cout << "Attack outcome: " << toString(summary) << std::endl;
        print_description(describe(hero));
        print_description(describe(monster));
        assert(false && "SafeToAttack");
      }
    }

    inline void safeAttack(Hero& hero, Monster& monster)
    {
      [[maybe_unused]] const auto summary = Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
      assert(!(summary == Summary::Death || summary == Summary::Petrified || summary == Summary::NotPossible));
    }

    inline void emplace_apply(Uncover uncover, Solution& solution, GameState& state)
    {
      if (!solution.empty())
      {
        if (auto existingUncover = std::get_if<Uncover>(&solution.back()))
        {
          existingUncover->numTiles += uncover.numTiles;
          solver::apply(uncover, state);
          return;
        }
      }
      solver::apply(uncover, state);
      solution.emplace_back(std::move(uncover));
    }

    inline void emplace_apply(Step step, Solution& solution, GameState& state)
    {
      solver::apply(step, state);
      solution.emplace_back(std::move(step));
    }

    inline void extendSolution(Solution nextSteps, Solution& solution, GameState& state)
    {
      solver::apply(nextSteps, state);
      solution.reserve(solution.size() + nextSteps.size());
      std::move(begin(nextSteps), end(nextSteps), std::back_inserter(solution));
    }
  } // namespace

  Solution buildLevelCatapult(GameState state)
  {
    Solution solution;
    const auto initialLevel = state.hero.getLevel();

    auto applyAttack = [&](size_t targetIndex) {
      emplace_apply(ChangeTarget{targetIndex}, solution, state);
      emplace_apply(Attack{}, solution, state);
      assert(!state.hero.isDefeated());
      return state.hero.getLevel() > initialLevel;
    };

    auto loopMonsters = [&](auto evalOneShotResult) {
      for (size_t n = 0; n < state.visibleMonsters.size();)
      {
        auto& monster = state.visibleMonsters[n];
        if (monster.grantsXP() && !monster.isDefeated())
        {
          const auto oneShotResult = checkOneShot(state.hero, monster);
          if (evalOneShotResult(oneShotResult))
          {
            if (oneShotResult == OneShotResult::VictoryGetindareOnly)
              emplace_apply(Cast{Spell::Getindare}, solution, state);
            if (applyAttack(n))
              return true;
            continue;
          }
        }
        ++n;
      }
      return false;
    };

    // Try flawless level catapult first
    if (loopMonsters([](auto result) { return result == OneShotResult::VictoryFlawless; }))
      return solution;

    // Allow taking damage / losing death protection
    if (loopMonsters([](auto result) { return result != OneShotResult::None && result != OneShotResult::Danger; }))
      return solution;

    return {};
  }

  bool checkMeleeOnly(Hero hero, Monster monster)
  {
    // Deny dodging
    hero.add(HeroStatus::Pessimist);

    while (!hero.isDefeated())
    {
      if (monster.isDefeated())
        return true;
      Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
    }
    return false;
  }

  inline bool canRecover(const Hero& hero)
  {
    return hero.getHitPoints() < hero.getHitPointsMax() && !hero.has(HeroDebuff::Poisoned) &&
           !hero.has(HeroStatus::Manaform) && (!hero.has(HeroTrait::Herbivore) || hero.getFoodCount() > 0);
  }

  inline void doRecovery(Hero& hero, Monster& monster, Solution& solution)
  {
    hero.recover(1u, ignoreMonsters);
    monster.recover(1u);
    if (!solution.empty())
    {
      if (auto uncover = std::get_if<Uncover>(&solution.back()))
      {
        ++(uncover->numTiles);
        return;
      }
    }
    solution.emplace_back(Uncover{1});
  };

  Solution checkRegenFight(Hero hero, Monster monster)
  {
    if (hero.isDefeated() || monster.isDefeated())
      return {};

    // Deny dodging
    hero.add(HeroStatus::Pessimist);

    // If recovery is needed to win the fight, it is always(?) better to do it first
    Solution solution;
    while (canRecover(hero) && !checkMeleeOnly(hero, monster))
      doRecovery(hero, monster, solution);

    std::optional<uint16_t> lowestMonsterHpOnAttack;

    auto maxIterations = 200;
    do
    {
      switch (checkOneShot(hero, monster))
      {
      case OneShotResult::None:
      {
        // confirm that we're making progress
        const auto monsterHitPoints = monster.getHitPoints();
        if (lowestMonsterHpOnAttack && monsterHitPoints >= *lowestMonsterHpOnAttack)
          return {};
        lowestMonsterHpOnAttack = monsterHitPoints;
        safeAttack(hero, monster);
        solution.emplace_back(Attack{});
        if (monster.isDefeated())
          return solution;
        break;
      }
      case OneShotResult::VictoryFlawless:
      case OneShotResult::VictoryDamaged:
      case OneShotResult::VictoryDeathProtectionLost:
        solution.emplace_back(Attack{});
        return solution;
      case OneShotResult::VictoryGetindareOnly:
      case OneShotResult::Danger:
        if (!canRecover(hero))
          return {};
        doRecovery(hero, monster, solution);
      }
    } while (--maxIterations);
    return {};
  }

  RegenFightResult toRegenFightResult(const Solution& solution)
  {
    unsigned numSquares{0};
    unsigned numAttacks{0};
    unsigned numAttacksBeforeCatapult{0};
    bool beforeCatapult{true};
    for (const auto& step : solution)
    {
      if (const auto uncover = std::get_if<Uncover>(&step))
        numSquares += uncover->numTiles;
      else if (std::get_if<Attack>(&step))
      {
        ++numAttacks;
        if (beforeCatapult)
          ++numAttacksBeforeCatapult;
      }
      else if (std::get_if<NoOp>(&step))
        beforeCatapult = false;
    }
    if (beforeCatapult) // Solution has no catapult
      return {.numAttacks = numAttacks, .numSquares = numSquares, .numAttacksBeforeCatapult = -1u};
    else
      return {.numAttacks = numAttacks, .numSquares = numSquares, .numAttacksBeforeCatapult = numAttacksBeforeCatapult};
  }

  std::optional<RegenFightResult> checkRegenFightFast(const Hero& hero, const Monster& monster)
  {
    const auto heroHpLossPerAttack = hero.predictDamageTaken(monster.getDamage(), monster.damageType());
    const auto burnStackSize = monster.getBurnStackSize();
    const auto monsterHpLossPerAttack =
        monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType()) - burnStackSize;
    const bool heroInitiative = hero.hasInitiativeVersus(monster);
    const bool justOneHit = monsterHpLossPerAttack + burnStackSize >= monster.getHitPoints();
    if (heroInitiative && justOneHit)
      return {{.numAttacks = 1}};
    if (monsterHpLossPerAttack == 0u)
      return {};
    if (heroHpLossPerAttack == 0u)
      return {{.numAttacks = static_cast<unsigned>(ceil(1.0 * monster.getHitPoints() / monsterHpLossPerAttack))}};

    // Evaluate initial combat, before any recovery is needed
    const auto numAttacksBeforeRecovery = (hero.getHitPoints() - 1) / heroHpLossPerAttack;
    auto heroHitPoints = hero.getHitPoints() - heroHpLossPerAttack * numAttacksBeforeRecovery;
    auto monsterTotalHpLoss = monsterHpLossPerAttack * numAttacksBeforeRecovery + burnStackSize;
    if (heroInitiative && hero.hasInitiativeVersusIgnoreMonsterSlowed(monster))
      monsterTotalHpLoss += monsterHpLossPerAttack;
    if (monsterTotalHpLoss >= monster.getHitPoints())
      return {{.numAttacks = numAttacksBeforeRecovery}};
    if (hero.has(HeroDebuff::Poisoned) || hero.has(HeroStatus::Manaform))
      return {};

    // Setup regen fighting evaluation
    const auto heroRecoveryRate = hero.recoveryMultiplier();
    const auto monsterRecoveryRate = monster.getLevel() * (monster.has(MonsterTrait::FastRegen) ? 2u : 1u);
    auto monsterPoison = monster.getPoisonAmount();
    if (monsterPoison == 0 && monsterHpLossPerAttack * heroRecoveryRate < heroHpLossPerAttack * monsterRecoveryRate)
      return {};

    auto recover = [&] {
      auto uncoverForNextAttack =
          static_cast<unsigned>(ceil(1.0 * (heroHpLossPerAttack + 1 - heroHitPoints) / heroRecoveryRate));
      heroHitPoints += uncoverForNextAttack * heroRecoveryRate;
      if (monsterPoison < uncoverForNextAttack)
      {
        if (monsterPoison > 0)
        {
          uncoverForNextAttack -= monsterPoison;
          monsterPoison = 0;
        }
        const auto recoveredHitPoints = uncoverForNextAttack * monsterRecoveryRate;
        if (recoveredHitPoints >= monsterTotalHpLoss)
        {
          // Not winnable; can only get here for poisoned monster
          monsterTotalHpLoss = 0;
          return 1000u;
        }
        monsterTotalHpLoss -= uncoverForNextAttack * monsterRecoveryRate;
      }
      else
      {
        monsterPoison -= uncoverForNextAttack;
      }
      return uncoverForNextAttack;
    };

    RegenFightResult result{.numAttacks = numAttacksBeforeRecovery};

    // Simulate regen fighting
    do
    {
      if (heroHitPoints <= heroHpLossPerAttack)
      {
        result.numSquares += recover();
        if (result.numSquares > 400u)
          return {};
      }
      heroHitPoints -= heroHpLossPerAttack;
      monsterTotalHpLoss += monsterHpLossPerAttack;
    } while (monsterTotalHpLoss < monster.getHitPoints());
    return {std::move(result)};
  }

  Solution checkRegenFightWithCatapult(Hero hero, Monster monster)
  {
    Solution solution;
    while (isSafeToAttack(hero, monster))
    {
      safeAttack(hero, monster);
      solution.emplace_back(Attack{});
      if (monster.isDefeated())
        return solution;
    }

    const auto levelUpAndFight = [](Hero hero, Monster monster, Solution initialSolution) {
      hero.gainLevel(ignoreMonsters);
      if (const auto regenSolution = checkRegenFight(std::move(hero), std::move(monster)); !regenSolution.empty())
      {
        auto& combinedSolution = initialSolution;
        combinedSolution.emplace_back(NoOp{});
        combinedSolution.insert(end(combinedSolution), begin(regenSolution), end(regenSolution));
        return combinedSolution;
      }
      return Solution{};
    };

    // It may be worthwhile to recover just enough to get one more hit in before levelling
    const auto solutionA = [&, hero, monster, solution]() mutable -> Solution {
      const auto monsterHpBeforeRecovery = monster.getHitPoints();
      while (!isSafeToAttack(hero, monster))
      {
        if (!canRecover(hero))
          return {};
        doRecovery(hero, monster, solution);
      }
      safeAttack(hero, monster);
      if (monster.getHitPoints() >= monsterHpBeforeRecovery)
        return {};
      solution.emplace_back(Attack{});
      if (monster.isDefeated())
        return solution;
      return levelUpAndFight(std::move(hero), std::move(monster), std::move(solution));
    }();

    const auto solutionB = levelUpAndFight(std::move(hero), std::move(monster), std::move(solution));

    const auto countTiles = [](const Solution& solution) {
      return std::transform_reduce(begin(solution), end(solution), 0u, std::plus<unsigned>(), [](const Step& step) {
        if (const auto uncover = std::get_if<Uncover>(&step))
          return uncover->numTiles;
        return 0u;
      });
    };

    if (!solutionA.empty() && (solutionB.empty() || countTiles(solutionA) < countTiles(solutionB)))
      return solutionA;
    return solutionB;
  }

  Solution checkRegenFightWithCatapult(GameState state)
  {
    Solution solution;

    // If recovery is needed to win the fight, it is always(?) better to do it first
    while (canRecover(state.hero) && !checkMeleeOnly(state.hero, state.visibleMonsters[state.activeMonster]))
      emplace_apply(Uncover{1}, solution, state);

    while (isSafeToAttack(state.hero, state.visibleMonsters[state.activeMonster]))
    {
      emplace_apply(Attack{}, solution, state);
      if (state.visibleMonsters[state.activeMonster].isDefeated())
        return solution;
    }

    auto levelUpAndFight = [](GameState state, Solution solution) -> Solution {
      const auto activeMonsterID = state.visibleMonsters[state.activeMonster].getID();
      auto catapultSolution = buildLevelCatapult(state);
      if (catapultSolution.empty())
        return {};
      extendSolution(std::move(catapultSolution), solution, state);
      auto activeMonster =
          std::find_if(begin(state.visibleMonsters), end(state.visibleMonsters),
                       [activeMonsterID](const auto& monster) { return monster.getID() == activeMonsterID; });
      assert(activeMonster != end(state.visibleMonsters));
      if (const auto regenSolution = checkRegenFight(std::move(state.hero), std::move(*activeMonster));
          !regenSolution.empty())
      {
        solution.reserve(solution.size() + regenSolution.size() + 1);
        const auto targetIndex = static_cast<size_t>(std::distance(begin(state.visibleMonsters), activeMonster));
        // For readability, skip change target instruction if we're just attacking monsters at index 0
        if (!(targetIndex == 0 && state.activeMonster == 0))
          solution.emplace_back(ChangeTarget{targetIndex});
        std::move(begin(regenSolution), end(regenSolution), std::back_inserter(solution));
        return solution;
      }
      return {};
    };

    // It may be worthwhile to recover just enough to get one more hit in before levelling
    const auto solutionA = [&levelUpAndFight](GameState state, Solution solution) mutable -> Solution {
      auto& activeMonster = state.visibleMonsters[state.activeMonster];
      const auto monsterHpBeforeRecovery = activeMonster.getHitPoints();
      while (!isSafeToAttack(state.hero, activeMonster))
      {
        if (!canRecover(state.hero))
          return {};
        emplace_apply(Uncover{1}, solution, state);
      }
      assertSafeToAttack(state.hero, activeMonster);
      emplace_apply(Attack{}, solution, state);
      assert(!state.hero.isDefeated());
      if (activeMonster.getHitPoints() >= monsterHpBeforeRecovery)
        return {};
      if (activeMonster.isDefeated())
        return solution;
      return levelUpAndFight(std::move(state), std::move(solution));
    }(state, solution);

    const auto solutionB = levelUpAndFight(std::move(state), std::move(solution));

    const auto countTiles = [](const Solution& solution) {
      return std::transform_reduce(begin(solution), end(solution), 0u, std::plus<unsigned>(), [](const Step& step) {
        if (const auto uncover = std::get_if<Uncover>(&step))
          return uncover->numTiles;
        return 0u;
      });
    };

    if (!solutionA.empty() && (solutionB.empty() || countTiles(solutionA) < countTiles(solutionB)))
      return solutionA;
    return solutionB;
  }
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState state)
{
  using namespace heuristics;
  Solution solution;

  bool progress = true;
  while (progress)
  {
    progress = false;
    for (auto monsterIndex : sorted_by_level_index(state.visibleMonsters))
    {
      const auto& monster = state.visibleMonsters[monsterIndex];
      state.activeMonster = monsterIndex;
      std::cout << "Trying to defeat " << monster.getName() << " ... ";

      auto regenFightResult = checkRegenFightWithCatapult(state);
      if (!regenFightResult.empty())
      {
        solution.emplace_back(ChangeTarget{monsterIndex});
        extendSolution(regenFightResult, solution, state);
        std::cout << "SUCCESS" << std::endl;
        progress = true;
        break;
      }
      else
      {
        std::cout << "FAILED" << std::endl;
      }
    }
  }
  return solution;
}
