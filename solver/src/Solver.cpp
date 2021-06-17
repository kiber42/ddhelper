#include "solver/Solver.hpp"

#include "engine/Combat.hpp"

#include <algorithm>
#include <cassert>
#include <execution>
#include <iostream>
#include <random>

static std::mt19937 generator(std::random_device{"/dev/urandom"}());

using namespace solver;

namespace GeneticAlgorithm
{
  // Random initial solution, stops close to hero's death
  Solution initialSolution(GameState state)
  {
    if (state.hero.isDefeated() || state.monsters.empty())
      return {};
    Solution initial;
    while (true)
    {
      Step step = generateRandomValidStep(state);
      assert(isValid(step, state));
      state = solver::apply(step, std::move(state));
      if (state.hero.isDefeated())
        break;
      initial.emplace_back(std::move(step));
      if (initial.size() == 100 || state.monsters.empty())
        break;
    }
    return initial;
  }

  int fitnessScore(const GameState& finalState)
  {
    if (finalState.monsters.empty())
      return 1000;
    const auto& hero = finalState.hero;
    const auto heroScore = hero.getLevel() * 50 + hero.getXP() + hero.getDamageVersusStandard() + hero.getHitPoints();
    return std::accumulate(
        begin(finalState.monsters), end(finalState.monsters), static_cast<int>(heroScore),
        [](const int runningTotal, const Monster& monster) { return runningTotal - static_cast<int>(monster.getHitPoints()); });
  }

  void explainScore(const GameState& finalState)
  {
    if (finalState.monsters.empty())
    {
      std::cout << "No monsters remaining, score = 1000" << std::endl;
      return;
    }
    const auto& hero = finalState.hero;
    const auto heroScore = hero.getLevel() * 50 + hero.getXP() + hero.getDamageVersusStandard() + hero.getHitPoints();
    std::cout << "   Hero level = " << hero.getLevel() << " -> " << 50 * hero.getLevel() << std::endl
              << "   Hero XP = " << hero.getXP() << std::endl
              << "   Hero damage = " << hero.getDamageVersusStandard() << std::endl
              << "   Hero hitpoints = " << hero.getHitPoints() << std::endl
              << "=> " << heroScore << std::endl;
    const auto monsterHitpoints = std::accumulate(
        begin(finalState.monsters), end(finalState.monsters), 0u,
        [](const unsigned runningTotal, const Monster& monster) { return runningTotal + monster.getHitPoints(); });
    std::cout << "   Total monster hit points = " << monsterHitpoints << std::endl
              << "=> " << heroScore - monsterHitpoints << std::endl;
  }

  // Applies mutations to a candidate solution, removes invalid steps and extends it with valid random steps.
  // Stops when the hero would be defeated by the next action.  Returns updated state.
  Solution mutateAndClean(Solution candidate, GameState state)
  {
    // The following probabilities are interpreted per step of current candidate.
    // The mutations are applied in this order:
    // 1) random erasure of a single step
    const double probability_erasure = 0.01;
    // 2) swap pairs of (neighbouring) steps
    const double probability_swap_any = 0.02;
    const double probability_swap_neighbor = 0.05;
    // 3) insert random valid step at random position
    const double probability_insert = 0.015;

    int num_mutations = std::poisson_distribution<>(candidate.size() * probability_erasure)(generator);
    while (--num_mutations >= 0 && !candidate.empty())
    {
      const int pos = std::uniform_int_distribution<>(0, static_cast<int>(candidate.size()) - 1)(generator);
      candidate.erase(begin(candidate) + pos);
    }
    if (!candidate.empty())
    {
      num_mutations = std::poisson_distribution<>(candidate.size() * probability_swap_any)(generator);
      while (--num_mutations >= 0)
      {
        const size_t posA = std::uniform_int_distribution<size_t>(0, candidate.size() - 1)(generator);
        const size_t posB = std::uniform_int_distribution<size_t>(0, candidate.size() - 1)(generator);
        if (posA != posB)
          std::swap(candidate[posA], candidate[posB]);
      }
    }
    if (candidate.size() >= 2)
    {
      num_mutations = std::poisson_distribution<>(candidate.size() * probability_swap_neighbor)(generator);
      while (--num_mutations >= 0)
      {
        const size_t pos = std::uniform_int_distribution<size_t>(0, candidate.size() - 2)(generator);
        std::swap(candidate[pos], candidate[pos + 1]);
      }
    }

    auto rand = std::uniform_real_distribution<double>();
    Solution cleanedSolution;
    cleanedSolution.reserve(static_cast<size_t>(candidate.size() * (1 + 3 * probability_insert)));
    for (auto& step : candidate)
    {
      if (!isValid(step, state))
        continue;
      state = solver::apply(step, std::move(state));
      if (state.hero.isDefeated())
        break;
      cleanedSolution.emplace_back(std::move(step));
      if (state.monsters.empty())
        break;
      if (rand(generator) < probability_insert)
      {
        auto randomStep = generateRandomValidStep(state);
        state = solver::apply(randomStep, std::move(state));
        if (state.hero.isDefeated())
          break;
        cleanedSolution.emplace_back(std::move(randomStep));
        if (state.monsters.empty())
          break;
      }
    }
    if (!state.hero.isDefeated() && !state.monsters.empty())
    {
      while (true)
      {
        auto randomStep = generateRandomValidStep(state);
        state = solver::apply(randomStep, std::move(state));
        if (state.hero.isDefeated())
          break;
        cleanedSolution.emplace_back(randomStep);
        if (state.monsters.empty())
          break;
      }
    }
    return cleanedSolution;
  }

  std::optional<Solution> run(GameState state)
  {
    state.hero.addStatus(HeroStatus::Pessimist);
    const unsigned num_generations = 100;
    const unsigned generation_size = 1000;
    // Keep `num_keep` top performers, multiply them to reach original generation size
    const unsigned num_keep = 100;

    // Create and rate initial generation of solutions
    std::array<std::pair<Solution, int>, generation_size> population;
    std::generate(begin(population), end(population), [&state] {
      auto candidate = initialSolution(state);
      const auto finalState = solver::apply(candidate, state);
      return std::pair{std::move(candidate), fitnessScore(finalState)};
    });

    for (unsigned gen = 0; gen < num_generations; ++gen)
    {
      std::stable_sort(begin(population), end(population),
                       [](const auto& scoredCandidateA, const auto& scoredCandidateB) {
                         return scoredCandidateA.second > scoredCandidateB.second;
                       });

      std::cout << "Generation " << gen << " complete:" << std::endl;
      std::cout << "  Highest fitness score: " << population.front().second << std::endl;
      std::cout << "  Lowest retained fitness score: " << population[num_keep - 1].second << std::endl;
      const auto& bestCandidateSolution = population.front().first;
      std::cout << "  Best candidate: " << std::endl << "  " << toString(bestCandidateSolution) << std::endl;
      explainScore(apply(bestCandidateSolution, state));
      std::cout << std::string(80, '-') << std::endl;

      if (population.front().second == 1000)
        return population.front().first;

      // A) Spawn new generation of candidate solutions by mixing successful solutions
      // Fill entire array with copies of `num_keep` most successful solutions
      auto n = num_keep;
      while (n < generation_size)
      {
        const auto num_copy = std::min(num_keep, generation_size - n);
        std::copy(begin(population), begin(population) + num_copy, begin(population) + n);
        n += num_keep;
      }

      // Shuffle, but always keep (one) best solution at the first position
      std::shuffle(begin(population) + 1, end(population), generator);

      // Generate new solution candidates by intertwining two existing candidates
      for (unsigned j = 2; j < generation_size; j += 2)
      {
        auto& solutionA = population[j - 1].first;
        auto& solutionB = population[j].first;
        const auto maxSize = std::max(solutionA.size(), solutionB.size());
        if (solutionA.size() < maxSize)
          solutionA.resize(maxSize);
        else
          solutionB.resize(maxSize);
        const int cutPosition = std::uniform_int_distribution<>(0, maxSize)(generator);
        // Replace from start up to random cut position, so vector sizes do not need to be changed
        std::vector<Step> tempSegment{begin(solutionA), begin(solutionA) + cutPosition};
        std::copy(begin(solutionB), begin(solutionB) + cutPosition, begin(solutionA));
        std::copy(begin(tempSegment), end(tempSegment), begin(solutionB));
      }
      // Run in parallel:
      // B) Random mutations
      // C) Clean up solutions and update scores
      std::for_each(std::execution::par_unseq, begin(population) + 1, end(population), [&](auto& entry) {
        auto cleaned = mutateAndClean(std::move(entry.first), state);
        const auto finalState = solver::apply(cleaned, state);
        entry = {std::move(cleaned), fitnessScore(finalState)};
      });
    }

    auto& best = std::max_element(begin(population), end(population), [](const auto& a, const auto& b) {
                   return a.second < b.second;
                 })->first;
    const auto finalState = solver::apply(best, state);
    explainScore(finalState);
    return std::move(best);
  }
} // namespace GeneticAlgorithm

std::optional<Solution> run(Solver solver, GameState initialState)
{
  switch (solver)
  {
  case Solver::GeneticAlgorithm:
    return GeneticAlgorithm::run(std::move(initialState));
  }
}
