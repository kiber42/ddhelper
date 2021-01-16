#include "Solver.hpp"

#include "Combat.hpp"

#include <algorithm>
#include <cassert>
#include <execution>
#include <random>

static std::mt19937 generator(std::random_device{"/dev/urandom"}());

using namespace solver;

namespace GeneticAlgorithm
{
  // Random initial solution, stops close to hero's death
  Solution initialSolution(SolverState state)
  {
    if (state.hero.isDefeated() || state.pool.empty())
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
      if (initial.size() == 100 || state.pool.empty())
        break;
    }
    return initial;
  }

  int fitnessScore(const SolverState& finalState)
  {
    if (finalState.pool.empty())
      return 1000;
    const auto& hero = finalState.hero;
    const int heroScore = hero.getLevel() * 50 + hero.getXP() + hero.getDamageVersusStandard() + hero.getHitPoints();
    return std::accumulate(
        begin(finalState.pool), end(finalState.pool), heroScore,
        [](const int runningTotal, const Monster& monster) { return runningTotal - monster.getHitPoints(); });
  }

  void explainScore(const SolverState& finalState)
  {
    if (finalState.pool.empty())
    {
      std::cout << "No monsters remaining, score = 1000" << std::endl;
      return;
    }
    const auto& hero = finalState.hero;
    const int heroScore = hero.getLevel() * 50 + hero.getXP() + hero.getDamageVersusStandard() + hero.getHitPoints();
    std::cout << "   Hero level = " << hero.getLevel() << " -> " << 50 * hero.getLevel() << std::endl
              << "   Hero XP = " << hero.getXP() << std::endl
              << "   Hero damage = " << hero.getDamageVersusStandard() << std::endl
              << "   Hero hitpoints = " << hero.getHitPoints() << std::endl
              << "=> " << heroScore << std::endl;
    const int monsterHitpoints = std::accumulate(
        begin(finalState.pool), end(finalState.pool), 0,
        [](const int runningTotal, const Monster& monster) { return runningTotal + monster.getHitPoints(); });
    std::cout << "   Total monster hit points = " << monsterHitpoints << std::endl
              << "=> " << heroScore - monsterHitpoints << std::endl;
  }

  // Applies mutations to a candidate solution, removes invalid steps and extends it with valid random steps.
  // Stops when the hero would be defeated by the next action.  Returns updated state.
  Solution mutateAndClean(Solution candidate, SolverState state)
  {
    // The following probabilities are interpreted per step of current candidate.
    // The mutations are applied in this order:
    // 1) random erasure of a single step
    const double probability_erasure = 0.02;
    // 2) swap pairs of (neighbouring) steps
    const double probability_swap_any = 0.05;
    const double probability_swap_neighbor = 0.1;
    // 3) insert random step at random position (invalid steps will be automatically removed later)
    const double probability_insert = 0.1;

    int num_mutations = std::poisson_distribution<>(candidate.size() * probability_erasure)(generator);
    while (--num_mutations >= 0 && !candidate.empty())
    {
      const size_t pos = std::uniform_int_distribution<>(0, candidate.size() - 1)(generator);
      candidate.erase(begin(candidate) + pos);
    }
    if (!candidate.empty())
    {
      num_mutations = std::poisson_distribution<>(candidate.size() * probability_swap_any)(generator);
      while (--num_mutations >= 0)
      {
        const size_t posA = std::uniform_int_distribution<>(0, candidate.size() - 1)(generator);
        const size_t posB = std::uniform_int_distribution<>(0, candidate.size() - 1)(generator);
        if (posA != posB)
          std::swap(candidate[posA], candidate[posB]);
      }
    }
    if (candidate.size() >= 2)
    {
      num_mutations = std::poisson_distribution<>(candidate.size() * probability_swap_neighbor)(generator);
      while (--num_mutations >= 0)
      {
        const size_t pos = std::uniform_int_distribution<>(0, candidate.size() - 2)(generator);
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
      if (rand(generator) < probability_insert)
      {
        auto randomStep = generateRandomValidStep(state);
        state = solver::apply(randomStep, std::move(state));
        if (state.hero.isDefeated())
          break;
        cleanedSolution.emplace_back(std::move(randomStep));
      }
    }
    if (!state.hero.isDefeated())
    {
      while (true)
      {
        auto randomStep = generateRandomValidStep(state);
        state = solver::apply(randomStep, std::move(state));
        if (state.hero.isDefeated())
          break;
        cleanedSolution.emplace_back(randomStep);
      }
    }
    return cleanedSolution;
  }

  std::optional<Solution> run(SolverState state)
  {
    state.hero.addStatus(HeroStatus::Pessimist);
    const int num_generations = 100;
    const int generation_size = 1000;
    // Keep `num_keep` top performers, multiply them to reach original generation size
    const int num_keep = 100;

    // Create and rate initial generation of solutions
    std::array<std::pair<Solution, int>, generation_size> population;
    std::generate(begin(population), end(population), [&state] {
      auto candidate = initialSolution(state);
      const auto finalState = solver::apply(candidate, state);
      return std::pair{std::move(candidate), fitnessScore(finalState)};
    });

    for (int i = 0; i < num_generations; ++i)
    {
      std::stable_sort(begin(population), end(population),
                       [](const auto& scoredCandidateA, const auto& scoredCandidateB) {
                         return scoredCandidateA.second > scoredCandidateB.second;
                       });

      std::cout << "Generation " << i << " complete:" << std::endl;
      std::cout << "  Highest fitness score: " << population.front().second << std::endl;
      std::cout << "  Lowest retained fitness score: " << population[num_keep - 1].second << std::endl;
      const auto& bestCandidateSolution = population.front().first;
      std::cout << "  Best candidate: " << std::endl << "  " << toString(bestCandidateSolution) << std::endl;
      explainScore(apply(bestCandidateSolution, state));
      std::cout << std::string(80, '-') << std::endl;

      if (population.front().second == 1000)
        break;

      // A) Spawn new generation of candidate solutions by mixing successful solutions
      // Fill entire array with copies of `num_keep` most successful solutions
      int n = num_keep;
      while (n < generation_size)
      {
        const int num_copy = std::min(num_keep, generation_size - n);
        std::copy(begin(population), begin(population) + num_copy, begin(population) + n);
        n += num_keep;
      }

      // Shuffle, but always keep (one) best solution at the first position
      std::shuffle(begin(population) + 1, end(population), generator);

      // Generate new solution candidates by intertwining two existing candidates
      for (int j = 2; j < generation_size; j += 2)
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

namespace SimulatedAnnealing
{
  struct ExtraPotions
  {
    int health{0};
    int mana{0};
  };

  struct IntermediateResult
  {
    Solution solution;
    ExtraPotions extraPotions;
  };

  // Attempt to find solution that defeats one monster (extends existing solution)
  bool solve_1(Monster monster, Hero& hero, Resources& resources, IntermediateResult& result)
  {
    auto& solution = result.solution;
    if (monster.getHitPoints() >= monster.getHitPointsMax())
    {
      const int numSquares = std::min(hero.numSquaresForFullRecovery(), resources.numBlackTiles);
      if (numSquares > 0)
      {
        hero.recover(numSquares);
        resources.numBlackTiles -= numSquares;
        solution.emplace_back(Uncover{numSquares});
      }
    }

    auto tryGetSpell = [&](Spell spell) {
      if (hero.has(spell))
        return true;
      auto spellIt = std::find(begin(resources.spells), end(resources.spells), Spell::Burndayraz);
      if (spellIt == end(resources.spells))
        return false;
      resources.spells.erase(spellIt);
      solution.emplace_back(Find{spell});
      return true;
    };
    auto tryCast = [&](Spell spell) {
      const int costs = Magic::spellCosts(spell, hero);
      if (hero.getManaPoints() < costs)
        return false;
      if (!Magic::isPossible(hero, monster, spell))
        return false;
      if (!tryGetSpell(spell))
        return false;
      Magic::cast(hero, monster, spell);
      solution.emplace_back(Cast{spell});
      return true;
    };
    auto tryCastUsePotion = [&](Spell spell) {
      const int costs = Magic::spellCosts(spell, hero);
      if (hero.getManaPointsMax() < costs)
        return false;
      while (hero.getManaPoints() < costs && (hero.has(Item::ManaPotion) || result.extraPotions.mana != 0))
      {
        if (!hero.has(Item::ManaPotion))
          --result.extraPotions.mana;
        hero.use(Item::ManaPotion);
        solution.emplace_back(Use{Item::ManaPotion});
      }
      return tryCast(spell);
    };

    while (!monster.isDefeated())
    {
      const bool fullHealth = hero.getHitPoints() >= hero.getHitPointsMax();
      Hero originalHero(hero);
      Monster originalMonster(monster);
      const auto solutionSize = solution.size();
      if (!hero.hasInitiativeVersus(monster))
        tryCast(Spell::Getindare);
      tryCast(Spell::Bysseps);
      const Summary summary = Combat::attack(hero, monster);
      if (!hero.isDefeated())
      {
        solution.emplace_back(Attack{});
        continue;
      }

      // Undo attack and spells
      solution.resize(solutionSize);
      hero = std::move(originalHero);
      monster = std::move(originalMonster);

      if (!fullHealth && (hero.has(Item::HealthPotion) || result.extraPotions.health != 0))
      {
        if (!hero.has(Item::HealthPotion))
          --result.extraPotions.health;
        hero.use(Item::HealthPotion);
        solution.emplace_back(Use{Item::HealthPotion});
        continue;
      }
      if (!monster.doesMagicalDamage() && !hero.hasStatus(HeroStatus::Cursed) &&
          hero.getStatusIntensity(HeroStatus::StoneSkin) < 3 && tryCast(Spell::Endiswal))
        continue;
      if (tryCastUsePotion(Spell::Burndayraz))
      {
        if (hero.isDefeated())
          return false;
        continue;
      }
      return false;
    }
    assert(monster.isDefeated());
    assert(!hero.isDefeated());
    return true;
  }

  // Attempt to find solution with an infinite amount of health and mana potions available
  IntermediateResult solve_initial(SolverState state)
  {
    IntermediateResult result;
    result.extraPotions = {-1, -1};
    while (!state.pool.empty())
    {
      if (!solve_1(std::move(state.pool.back()), state.hero, state.resources, result))
        return {};
      state.pool.pop_back();
    }
    result.extraPotions = {-1 - result.extraPotions.health, -1 - result.extraPotions.mana};
    std::cout << "Intermediate solution used " << result.extraPotions.health << " health potion(s) and "
              << result.extraPotions.mana << " mana potion(s)\n";
    return result;
  }

  IntermediateResult useMoreMana(SolverState state, IntermediateResult previousResult)
  {
    IntermediateResult result;
    const ExtraPotions initialPotions = {previousResult.extraPotions.health - 1, previousResult.extraPotions.mana + 10};
    result.extraPotions = initialPotions;
    while (!state.pool.empty())
    {
      if (!solve_1(std::move(state.pool.back()), state.hero, state.resources, result))
        return {};
      state.pool.pop_back();
    }
    result.extraPotions = {initialPotions.health - result.extraPotions.health,
                           initialPotions.mana - result.extraPotions.mana};
    std::cout << "Intermediate solution used " << result.extraPotions.health << " health potion(s) and "
              << result.extraPotions.mana << " mana potion(s)\n";
    return result;
  }

  IntermediateResult reduce(SolverState state, IntermediateResult previousResult)
  {
    IntermediateResult result;

    // TODO

    return result;
  }

  std::optional<Solution> run(SolverState state)
  {
    state.hero.addStatus(HeroStatus::Pessimist);
    std::reverse(begin(state.pool), end(state.pool));

    auto result = solve_initial(state);
    if (result.solution.empty())
      return {};

    while (true)
    {
      auto previousResult = std::move(result);
      result = useMoreMana(state, previousResult);
      if (result.solution.empty())
      {
        result = std::move(previousResult);
        break;
      }
    }

    while (true)
    {
      auto previousResult = std::move(result);
      result = reduce(state, previousResult);
      if (result.solution.empty())
      {
        result = std::move(previousResult);
        break;
      }
    }

    return result.solution;
  }
} // namespace SimulatedAnnealing

std::optional<Solution> run(Solver solver, SolverState initialState)
{
  switch (solver)
  {
  case Solver::GeneticAlgorithm:
    return GeneticAlgorithm::run(std::move(initialState));
  case Solver::SimulatedAnnealing:
    return SimulatedAnnealing::run(std::move(initialState));
  }
}
