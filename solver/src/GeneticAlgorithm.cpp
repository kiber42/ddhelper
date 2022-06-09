#include "engine/Combat.hpp"
#include "engine/HeroStatus.hpp"
#include "engine/Resources.hpp"
#include "solver/Fitness.hpp"
#include "solver/SolverTools.hpp"

#include <algorithm>
#include <cassert>
#include <execution>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>

const auto fitnessRating = StateFitnessRating1{};

namespace
{
  using namespace solver;
  thread_local std::mt19937 generator{std::random_device{"/dev/urandom"}()};

  // Random initial solution, stops close to hero's death
  Solution initialSolution(GameState state)
  {
    if (state.hero.isDefeated() || state.visibleMonsters.empty())
      return {};
    Solution initial;
    while (true)
    {
      Step step = generateRandomValidStep(state, false);
      assert(isValid(step, state));
      solver::apply(step, state);
      if (state.hero.isDefeated())
        break;
      initial.emplace_back(std::move(step));
      if (initial.size() == 100 || state.visibleMonsters.empty())
        break;
    }
    return initial;
  }

  using OptionalStepResult = std::optional<std::pair<Step, GameState>>;

  // Generate and apply random step.  If hero survives, returns step and resulting gamestate; nullopt otherwise.
  OptionalStepResult makeRandomStep(GameState state, bool allowTargetChange)
  {
    auto randomStep = generateRandomValidStep(state, allowTargetChange);
    solver::apply(randomStep, state);
    if (!state.hero.isDefeated())
      return std::pair{std::move(randomStep), std::move(state)};
    return std::nullopt;
  }

  // Generate and apply several random steps.  Return the most successful one and the resulting gamestate, or nullopt
  // if the hero died in all attempted steps.
  OptionalStepResult bestRandomStep(GameState state, const StateFitnessRating& rate, int num_attempts = 3)
  {
    OptionalStepResult result;
    int bestRating;
    while (--num_attempts >= 0)
    {
      auto candidate = num_attempts > 0 ? makeRandomStep(state, false) : makeRandomStep(std::move(state), false);
      if (!candidate)
        continue;
      auto rating = rate(candidate->second);
      if (!result || rating > bestRating)
      {
        bestRating = rating;
        result = std::move(candidate);
      }
    }
    return result;
  }

  // Applies mutations to a candidate solution, removes invalid steps and extends it with valid random steps.
  // Stops when the hero would be defeated by the next action.  Returns updated state.
  Solution mutateAndClean(Solution candidate, GameState state, double temperature)
  {
    // The following probabilities are interpreted per step of current candidate.
    // The mutations are applied in this order:
    // 1) random erasure of a single step
    const double probability_erasure = 0.01 * temperature;
    // 2) swap pairs of (neighbouring) steps
    const double probability_swap_any = 0.01 * temperature;
    const double probability_swap_neighbor = 0.05 * temperature;
    // 3) insert random valid step at random position
    const double probability_insert = 0.04 * temperature;

    int num_mutations =
        std::poisson_distribution<>(static_cast<double>(candidate.size()) * probability_erasure)(generator);
    while (--num_mutations >= 0 && !candidate.empty())
    {
      const int pos = std::uniform_int_distribution<>(0, static_cast<int>(candidate.size()) - 1)(generator);
      candidate.erase(begin(candidate) + pos);
    }
    if (!candidate.empty())
    {
      num_mutations =
          std::poisson_distribution<>(static_cast<double>(candidate.size()) * probability_swap_any)(generator);
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
      num_mutations =
          std::poisson_distribution<>(static_cast<double>(candidate.size()) * probability_swap_neighbor)(generator);
      while (--num_mutations >= 0)
      {
        const size_t pos = std::uniform_int_distribution<size_t>(0, candidate.size() - 2)(generator);
        std::swap(candidate[pos], candidate[pos + 1]);
      }
    }

    auto rand = std::uniform_real_distribution<double>();
    Solution cleanedSolution;
    cleanedSolution.reserve(static_cast<size_t>(static_cast<double>(candidate.size()) * (1 + 3 * probability_insert)));
    for (auto& step : candidate)
    {
      if (!isValid(step, state))
        continue;
      solver::apply(step, state);
      if (state.hero.isDefeated())
        break;
      cleanedSolution.emplace_back(std::move(step));
      if (state.visibleMonsters.empty())
        break;
      if (rand(generator) < probability_insert)
      {
        auto bestRandom = bestRandomStep(std::move(state), fitnessRating, 5);
        if (!bestRandom)
          break;
        cleanedSolution.emplace_back(std::move(bestRandom->first));
        state = std::move(bestRandom->second);
        if (state.visibleMonsters.empty())
          break;
      }
    }
    if (!state.hero.isDefeated() && !state.visibleMonsters.empty())
    {
      while (true)
      {
        auto bestRandom = bestRandomStep(std::move(state), fitnessRating, 3);
        if (!bestRandom)
          break;
        cleanedSolution.emplace_back(std::move(bestRandom->first));
        state = std::move(bestRandom->second);
        if (state.visibleMonsters.empty())
          break;
      }
    }
    return cleanedSolution;
  }
} // namespace

std::optional<Solution> runGeneticAlgorithm(GameState state)
{
  state.hero.add(HeroStatus::Pessimist);
  const unsigned num_generations = 100;
  const unsigned generation_size = 1000;
  // Keep `num_keep` top performers, multiply them to reach original generation size
  const unsigned num_keep = 100;

  // Create and rate initial generation of solutions
  std::array<std::pair<Solution, int>, generation_size> population;
  bool initialized = false;
  int best_previous = 0;
  double temperature = 1;
  for (unsigned gen = 0; gen < num_generations; ++gen)
  {
    if (!initialized)
    {
      std::generate(std::execution::par_unseq, begin(population), end(population), [&state] {
        auto candidate = initialSolution(state);
        auto finalState = state;
        solver::apply(candidate, finalState);
        return std::pair{std::move(candidate), fitnessRating(finalState)};
      });
      initialized = true;
    }

    std::stable_sort(begin(population), end(population),
                     [](const auto& scoredCandidateA, const auto& scoredCandidateB) {
                       return scoredCandidateA.second > scoredCandidateB.second;
                     });

    const auto& [bestSolution, bestScore] = population.front();
    std::cout << "Generation " << gen << " complete:" << std::endl;
    std::cout << "  Highest fitness score: " << bestScore << std::endl;
    std::cout << "  Lowest retained fitness score: " << population[num_keep - 1].second << std::endl;
    std::cout << "  Current temperature: " << static_cast<int>(temperature * 100) << std::endl;
    std::cout << "  Best candidate: " << std::endl << "  " << toString(bestSolution) << std::endl;
    auto finalState = state;
    apply(bestSolution, finalState);
    fitnessRating.explain(finalState);
    std::cout << std::string(80, '-') << std::endl;
    if (bestScore == fitnessRating.GAME_WON)
      return bestSolution;

    if (bestScore <= best_previous)
      temperature += 0.1;
    else
      temperature /= 2;
    best_previous = bestScore;

    if (temperature > 3)
    {
      // start over
      initialized = false;
      temperature = 1;
      continue;
    }

    // A) Spawn new generation of candidate solutions by mixing successful solutions
    // Fill array with copies of `num_keep` most successful solutions
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
      const auto cutPosition = std::uniform_int_distribution<long>(0, static_cast<long>(maxSize))(generator);
      // Replace from start up to random cut position, so vector sizes do not need to be changed
      std::vector<Step> tempSegment{begin(solutionA), begin(solutionA) + cutPosition};
      std::copy(begin(solutionB), begin(solutionB) + cutPosition, begin(solutionA));
      std::copy(begin(tempSegment), end(tempSegment), begin(solutionB));
    }
    // Run in parallel:
    // B) Random mutations
    // C) Clean up solutions and update scores
    std::for_each(std::execution::par_unseq, begin(population) + 1, end(population), [&](auto& entry) {
      auto cleaned = mutateAndClean(std::move(entry.first), state, temperature);
      auto finalState = state;
      solver::apply(cleaned, finalState);
      entry = {std::move(cleaned), fitnessRating(finalState)};
    });
  }

  auto& best = std::max_element(begin(population), end(population), [](const auto& a, const auto& b) {
                 return a.second < b.second;
               })->first;
  auto finalState = state;
  solver::apply(best, finalState);
  fitnessRating.explain(finalState);
  return std::move(best);
}
