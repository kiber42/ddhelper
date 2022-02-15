#include "solver/Fitness.hpp"
#include "solver/Solver.hpp"
#include "solver/SolverTools.hpp"

#include <algorithm>
#include <execution>
#include <iostream>

namespace
{
  int rateSpell(const GameState& state, const Spell& spell)
  {
    assert(!state.visibleMonsters.empty());
    const auto& hero = state.hero;
    const auto& monster = state.visibleMonsters.front();
    switch (spell)
    {
    case Spell::Apheelsik:
      return 1;
    case Spell::Bludtupowa:
      return static_cast<int>(sqrt(state.resources.numHiddenTiles)) - hero.getManaPoints();
    case Spell::Burndayraz:
    {
      const bool heavy = hero.has(HeroStatus::HeavyFireball);
      const unsigned multiplier =
          4 + hero.has(Boon::Flames) + (heavy ? 4 : 0) + (hero.has(ShopItem::BattlemageRing) ? 1 : 0);
      const auto monsterDies =
          !monster.getDeathProtection() &&
          monster.predictDamageTaken(hero.getLevel() * multiplier, DamageType::Magical) >= monster.getHitPoints();
      if (monsterDies)
        return 100;
      const bool monsterSlowed = monster.isSlowed();
      const bool retaliate = monster.has(MonsterTrait::Retaliate);
      if (retaliate)
        return monsterSlowed ? 150 : -10;
      return 10;
    }
    case Spell::Bysseps:
    {
      const auto heroDamage = hero.getDamageOutputVersus(monster);
      const auto heroDamageType = hero.damageType();
      const auto monsterHP = monster.getHitPoints();
      const auto monsterDiesNoBysseps = monster.predictDamageTaken(heroDamage, heroDamageType) >= monsterHP;
      const auto monsterDiesWithBysseps = monster.predictDamageTaken(heroDamage * 13 / 10, heroDamageType) >= monsterHP;
      if (monsterDiesNoBysseps)
        return -20;
      if (!monsterDiesWithBysseps)
        return monster.getPhysicalResistPercent() + monster.getMagicalResistPercent() > 0 ? 10 : 0;
      return 20;
    }
    case Spell::Cydstepp:
      return 3;
    case Spell::Endiswal:
      return 3;
    case Spell::Getindare:
    {
      const auto monsterDies =
          !monster.getDeathProtection() &&
          monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType()) >= monster.getHitPoints();
      if (!monsterDies || hero.hasInitiativeVersus(monster))
        return 0;
      return 30;
    }
    case Spell::Halpmeh:
    {
      const auto monsterDies =
          !monster.getDeathProtection() &&
          monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType()) >= monster.getHitPoints();
      const auto heroWillBeHit = !hero.hasInitiativeVersus(monster) || !monsterDies;
      if (heroWillBeHit && hero.predictDamageTaken(monster.getDamage(), monster.damageType()) >= hero.getHitPoints())
        return 20;
      return 0;
    }
    case Spell::Imawal:
      if (hero.has(HeroStatus::ExperienceBoost))
        return -30;
      return static_cast<int>(10 + hero.getLevel() - 3 * monster.getLevel());
    case Spell::Lemmisi:
      return static_cast<int>(sqrt(state.resources.numHiddenTiles)) - 2;
    case Spell::Pisorf:
      return 5;
    case Spell::Weytwut:
    {
      const auto initiative = hero.hasInitiativeVersus(monster);
      const auto monsterDies =
          !monster.getDeathProtection() &&
          monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType()) >= monster.getHitPoints();
      // TODO: This ignores several status effects (determined, sanguine, ...)
      return monsterDies && !initiative ? 30 : -5;
    }
    case Spell::Wonafyt:
      return 0;
    }
  }

  int rateStep(const GameState& state, const Step& step)
  {
    assert(!state.visibleMonsters.empty());
    const auto& hero = state.hero;
    const auto& monster = state.visibleMonsters.front();
    return std::visit(
        overloaded{[&](Attack) {
                     const auto monsterDies = monster.predictDamageTaken(hero.getDamageOutputVersus(monster),
                                                                         hero.damageType()) >= monster.getHitPoints();
                     const auto heroWillBeHit = !hero.hasInitiativeVersus(monster) || !monsterDies;
                     // TODO: This ignores several status effects (determined, sanguine, ...)
                     if (heroWillBeHit &&
                         hero.predictDamageTaken(monster.getDamage(), monster.damageType()) >= hero.getHitPoints())
                       return -10000;
                     return monsterDies ? (heroWillBeHit ? 80 : 100) : -1;
                   },
                   [&](Cast cast) { return rateSpell(state, cast.spell); }, [](Uncover) { return 1; },
                   [](Buy) { return 1; }, [](Use) { return 3; }, [](Convert) { return 0; }, [](Find) { return 20; },
                   [](FindFree) { return 21; }, [](Follow) { return 8; }, [](Request) { return 4; },
                   [](Desecrate) { return -2; }, [](NoOp) { return 0; }},
        step);
  }

  using RatedSolution = std::pair<Solution, int>;

  // Finds best solution within the maximum allowed depth. Solution is in reverse order.
  RatedSolution search(const GameState& state, const StateFitnessRating& fitnessRating, int maxDepth, bool run_parallel)
  {
    if (state.hero.isDefeated())
      return {{}, fitnessRating.GAME_LOST};
    if (state.visibleMonsters.empty())
      return {{}, fitnessRating.GAME_WON};
    if (maxDepth == 0)
      return {{}, fitnessRating(state)};
    const auto steps = solver::generateAllValidSteps(state);
    if (run_parallel)
    {
      auto ratedSolutions = std::vector<RatedSolution>(steps.size());
      std::transform(std::execution::par_unseq, begin(steps), end(steps), begin(ratedSolutions), [&](Step step) {
        auto [solution, score] = search(solver::apply(step, std::move(state)), fitnessRating, maxDepth - 1, false);
        solution.push_back(step);
        return std::pair{std::move(solution), score};
      });

      auto bestIter = std::max_element(begin(ratedSolutions), end(ratedSolutions),
                                       [](const auto& a, const auto& b) { return a.second < b.second; });
      return *bestIter;
    }

    // Process steps in order of heuristic rating
    std::vector<std::pair<Step, int>> scoredSteps;
    scoredSteps.reserve(steps.size());
    for (std::size_t i = 0; i < steps.size(); ++i)
      scoredSteps.emplace_back(std::move(steps[i]), rateStep(state, steps[i]));
    std::sort(begin(scoredSteps), end(scoredSteps), [](const auto& a, const auto& b) { return a.second < b.second; });
    if (scoredSteps.size() > 5)
    {
      // TODO: This is much too crude, and the heuristic is not yet doing a good job
      scoredSteps.resize(scoredSteps.size() * 3 / 4);
    }

    int bestScore = fitnessRating.GAME_LOST;
    Solution bestSolution;
    for (auto& [step, _] : scoredSteps)
    {
      auto updated = solver::apply(step, state);
      auto [solution, score] = search(updated, fitnessRating, maxDepth - 1, false);
      if (score > bestScore)
      {
        bestScore = score;
        bestSolution = std::move(solution);
        bestSolution.push_back(std::move(step));
      }
      if (score == fitnessRating.GAME_WON)
        break;
    }
    return {std::move(bestSolution), bestScore};
  }
} // namespace

std::optional<Solution> runTreeSearch(GameState state)
{
  Solution solution{};
  auto fitness = StateFitnessRating1{};
  while (!state.visibleMonsters.empty())
  {
    auto [partialSolution, score] = search(state, fitness, 6, true);
    if (score == fitness.GAME_LOST)
      return std::nullopt;
    // partial solutions are returned in reverse order
    std::reverse(begin(partialSolution), end(partialSolution));
    std::cout << "------------------------------------------" << std::endl;
    solver::print(partialSolution, state);
    std::cout << "SCORE SO FAR: " << score << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    state = solver::apply(partialSolution, std::move(state));
    std::copy(begin(partialSolution), end(partialSolution), std::back_inserter(solution));
    assert(!state.hero.isDefeated());
  }
  return solution;
}
