#include "Solver.hpp"

#include "Combat.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>
#include <execution>
#include <random>

static std::mt19937 generator(std::random_device{"/dev/urandom"}());

namespace
{
  Step randomStep()
  {
    std::uniform_int_distribution<> randomAction(0, 11);
    std::uniform_int_distribution<> randomSpell(0, static_cast<int>(Spell::Last) - 1);
    std::uniform_int_distribution<> randomShopItem(0, static_cast<int>(Item::LastShopItem) - 1);
    std::uniform_int_distribution<> randomItem(0, static_cast<int>(Item::Last) - 1);
    std::uniform_int_distribution<> randomGod(0, static_cast<int>(God::Last) - 1);
    std::uniform_int_distribution<> randomBoon(0, static_cast<int>(Boon::Last) - 1);
    std::uniform_int_distribution<> randomPact(0, static_cast<int>(Pact::LastWithConsensus) - 1);
    switch (randomAction(generator))
    {
    default:
    case 0:
      return Attack{};
    case 1:
      return Cast{static_cast<Spell>(randomSpell(generator))};
    case 2:
      return Uncover{1};
    case 3:
      return Buy{static_cast<Item>(randomShopItem(generator))};
    case 4:
      return Use{static_cast<Item>(randomItem(generator))};
    case 5:
      return Convert{static_cast<Item>(randomItem(generator))};
    case 6:
      return Convert{static_cast<Spell>(randomSpell(generator))};
    case 7:
      return Find{static_cast<Spell>(randomSpell(generator))};
    case 8:
      return Follow{static_cast<God>(randomGod(generator))};
    case 9:
      return Request{static_cast<Boon>(randomBoon(generator))};
    case 10:
      return Request{static_cast<Pact>(randomPact(generator))};
    case 11:
      return Desecrate{static_cast<God>(randomGod(generator))};
    }
  }

  Step validRandomStep(const SolverState& state)
  {
    // TODO: Migrate to C++20 and replace std::transform and std::shuffle with equivalent std::ranges functions

    // Rely on at least one monster being present
    assert(!state.pool.empty());
    std::uniform_int_distribution<> randomAction(0, 10);
    std::optional<std::vector<Spell>> spells;
    std::optional<std::vector<Item>> shops;
    std::optional<std::vector<Item>> items;

    auto otherAltar = [&hero = state.hero, altars = state.resources.altars]() mutable -> std::optional<God> {
      if (altars.empty())
        return {};
      std::shuffle(begin(altars), end(altars), generator);
      if (!hero.getFollowedDeity())
        return altars.front();
      auto altarIt =
          std::find_if(begin(altars), end(altars),
                       [followedDeity = *hero.getFollowedDeity()](const auto altar) { return altar != followedDeity; });
      if (altarIt != end(altars))
        return *altarIt;
      return {};
    };

    auto randomElement = [](auto& vec) {
      assert(!vec.empty());
      return vec[std::uniform_int_distribution<>(0, vec.size() - 1)(generator)];
    };

    while (true)
    {
      switch (randomAction(generator))
      {
      default:
      case 0:
        return Attack{};
      case 1:
        if (!spells)
        {
          const auto spellEntries = state.hero.getSpells();
          spells.emplace(spellEntries.size());
          std::transform(begin(spellEntries), end(spellEntries), begin(*spells),
                         [](const auto& entry) { return std::get<Spell>(entry.itemOrSpell); });
          std::shuffle(begin(*spells), end(*spells), generator);
          const auto spellIt = std::find_if(begin(*spells), end(*spells),
                                            [&hero = state.hero, &monster = state.pool.front()](const Spell spell) {
                                              return Magic::isPossible(hero, monster, spell) ||
                                                     (!Magic::needsMonster(spell) && Magic::isPossible(hero, spell));
                                            });
          if (spellIt != end(*spells))
            return Cast{*spellIt};
        }
        break;
      case 2:
        if (state.resources.numBlackTiles > 0)
          return Uncover{1};
        break;
      case 3:
        if (!state.hero.hasRoomFor(Item::HealthPotion) || state.hero.gold() == 0)
          break;
        if (!shops)
        {
          shops = state.resources.shops;
          std::shuffle(begin(*shops), end(*shops), generator);
          const auto shopIt = std::find_if(begin(*shops), end(*shops), [&hero = state.hero](const Item item) {
            return hero.hasRoomFor(item) && hero.canAfford(item);
          });
          if (shopIt != end(*shops))
            return Buy{*shopIt};
        }
        break;
      case 4:
        if (!items)
        {
          const auto itemEntries = state.hero.getItems();
          items.emplace(itemEntries.size());
          std::transform(begin(itemEntries), end(itemEntries), begin(*items),
                         [](const auto& entry) { return std::get<Item>(entry.itemOrSpell); });
          std::shuffle(begin(*items), end(*items), generator);
          const auto itemIt = std::find_if(begin(*items), end(*items),
                                           [&hero = state.hero](const Item item) { return hero.canUse(item); });
          if (itemIt != end(*items))
            return Use{*itemIt};
        }
        break;
      case 5:
      {
        const auto& entries = state.hero.getItemsAndSpells();
        if (!entries.empty())
        {
          const auto& entry = randomElement(entries);
          if (entry.conversionPoints >= 0)
            return Convert{entry.itemOrSpell};
          // few items cannot be converted, do not retry here
        }
        break;
      }
      case 6:
        if (!state.resources.spells.empty() && state.hero.hasRoomFor(Spell::Burndayraz))
          return Find{randomElement(state.resources.spells)};
        break;
      case 7:
      {
        if (state.resources.altars.empty() || (state.hero.getFollowedDeity() && state.hero.getPiety() < 50))
          break;
        const auto altar = otherAltar();
        if (altar)
          return Follow{*altar};
        break;
      }
      case 8:
        if (state.hero.getFollowedDeity())
        {
          auto boons = offeredBoons(*state.hero.getFollowedDeity());
          std::shuffle(begin(boons), end(boons), generator);
          auto boonIt =
              std::find_if(begin(boons), end(boons), [&hero = state.hero, &faith = state.hero.getFaith()](Boon boon) {
                return faith.isAvailable(boon, hero);
              });
          if (boonIt != end(boons))
            return Request{*boonIt};
        }
        break;
      case 9:
        if (state.resources.pactMakerAvailable && !state.hero.getFaith().getPact())
        {
          const auto last = state.hero.getFaith().enteredConsensus() ? Pact::LastNoConsensus : Pact::LastWithConsensus;
          return Request{static_cast<Pact>(std::uniform_int_distribution<>(0, static_cast<int>(last) - 1)(generator))};
        }
        break;
      case 10:
        if (state.hero.getFollowedDeity() && !state.resources.altars.empty())
        {
          const auto altar = otherAltar();
          if (altar)
            return Desecrate{*altar};
        }
        break;
      }
    }
  }
} // namespace

namespace GeneticAlgorithm
{
  // Random initial solution; resulting state is returned as well
  std::pair<Solution, SolverState> initialSolution(SolverState state)
  {
    if (state.hero.isDefeated() || state.pool.empty())
      return {};
    Solution initial;
    while (true)
    {
      Step step = validRandomStep(state);
      assert(isValid(step, state));
      state = apply(std::move(step), std::move(state));
      if (state.hero.isDefeated())
        break;
      initial.emplace_back(step);
      if (initial.size() == 100 || state.pool.empty())
        break;
    }
    return {std::move(initial), std::move(state)};
  }

  // Runs a solution, removes invalid steps, adds random steps, and returns the resulting state
  std::pair<Solution, SolverState> cleanSolution(Solution candidate, SolverState state)
  {
    if (state.hero.isDefeated() || state.pool.empty())
      return {};
    std::reverse(begin(candidate), end(candidate));
    Solution cleaned;
    while (true)
    {
      while (!candidate.empty() && !isValid(candidate.back(), state))
        candidate.pop_back();
      Step step = candidate.empty() ? validRandomStep(state) : candidate.back();
      if (!candidate.empty())
        candidate.pop_back();
      state = apply(std::move(step), std::move(state));
      if (state.hero.isDefeated())
        break;
      cleaned.emplace_back(step);
      if (state.pool.empty())
        break;
    }
    return {std::move(cleaned), std::move(state)};
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

  void addRandomMutations(Solution& candidate)
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

    // Always add some random steps at the end (to guarantee minimum size and to facilitate finding longer solutions)
    for (int i = 0; i < 10; ++i)
      candidate.emplace_back(randomStep());

    int num_mutations = std::poisson_distribution<>(candidate.size() * probability_erasure)(generator);
    while (--num_mutations >= 0)
    {
      const size_t pos = std::uniform_int_distribution<>(0, candidate.size() - 1)(generator);
      candidate.erase(begin(candidate) + pos);
    }
    num_mutations = std::poisson_distribution<>(candidate.size() * probability_swap_any)(generator);
    while (--num_mutations >= 0)
    {
      const size_t posA = std::uniform_int_distribution<>(0, candidate.size() - 1)(generator);
      const size_t posB = std::uniform_int_distribution<>(0, candidate.size() - 1)(generator);
      if (posA != posB)
        std::swap(candidate[posA], candidate[posB]);
    }
    num_mutations = std::poisson_distribution<>(candidate.size() * probability_swap_neighbor)(generator);
    while (--num_mutations >= 0)
    {
      const size_t pos = std::uniform_int_distribution<>(0, candidate.size() - 2)(generator);
      std::swap(candidate[pos], candidate[pos + 1]);
    }
    num_mutations = std::poisson_distribution<>(candidate.size() * probability_insert)(generator);
    while (--num_mutations >= 0)
    {
      const size_t pos = std::uniform_int_distribution<>(0, candidate.size())(generator);
      candidate.insert(begin(candidate) + pos, randomStep());
    }
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
      const auto [candidate, finalState] = initialSolution(state);
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
        addRandomMutations(entry.first);
        auto [cleaned, finalState] = cleanSolution(entry.first, state);
        entry = {std::move(cleaned), fitnessScore(finalState)};
      });
    }

    const auto bestSolution = std::max_element(begin(population), end(population), [](const auto& a, const auto& b) {
                                return a.second < b.second;
                              })->first;
    const auto [bestSolutionCleaned, bestFinalState] = cleanSolution(bestSolution, state);
    explainScore(bestFinalState);
    return bestSolutionCleaned;
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

template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

SolverState apply(Step step, SolverState state)
{
  if (state.pool.empty())
    return state;
  auto& hero = state.hero;
  auto& monster = state.pool.front();
  std::visit(overloaded{[&](Attack) { Combat::attack(hero, monster); },
                        [&](Cast cast) { Magic::cast(hero, monster, cast.spell); },
                        [&](Uncover uncover) {
                          hero.recover(uncover.numTiles);
                          monster.recover(uncover.numTiles);
                        },
                        [&hero, &shops = state.resources.shops](Buy buy) {
                          shops.erase(std::find(begin(shops), end(shops), buy.item));
                          hero.buy(buy.item);
                        },
                        [&](Use use) { hero.use(use.item); },
                        [&](Convert convert) { hero.convert(convert.itemOrSpell); },
                        [&hero, &spells = state.resources.spells](Find find) {
                          spells.erase(std::find(begin(spells), end(spells), find.spell));
                          hero.receive(find.spell);
                        },
                        [&](Follow follow) { hero.followDeity(follow.deity); },
                        [&](Request request) { hero.request(request.boonOrPact); },
                        [&hero, &altars = state.resources.altars](Desecrate desecrate) {
                          altars.erase(std::find(begin(altars), end(altars), desecrate.altar));
                          hero.desecrate(desecrate.altar);
                        }},
             step);
  while (state.pool.front().isDefeated() && !state.pool.empty())
    state.pool.erase(state.pool.begin());
  return state;
}

SolverState apply(Solution solution, SolverState state)
{
  for (const auto& step : solution)
    state = apply(step, std::move(state));
  return state;
}

namespace
{
  void print_description(const std::vector<std::string>& description)
  {
    std::string s = std::accumulate(begin(description), end(description), std::string{},
                                    [](auto& a, auto& b) { return a + ", " + b; });
    if (!s.empty())
      std::cout << s.substr(2) << '\n';
  }
} // namespace

void print(Solution solution, SolverState state)
{
  print_description(describe(state.hero));
  if (!state.pool.empty())
    print_description(describe(state.pool.front()));
  for (const auto& step : solution)
  {
    std::cout << toString(step) << '\n';
    auto heroBefore = state.hero;
    state = apply(step, std::move(state));
    print_description(describe_diff(heroBefore, state.hero));
    if (isCombat(step) && !state.pool.empty())
    {
      print_description(describe(state.pool.front()));
      if (state.pool.front().isDefeated() && state.pool.size() >= 2)
        print_description(describe(state.pool[1]));
    }
  }
}

bool isValid(Step step, const SolverState& state)
{
  const auto monsterIt =
      std::find_if(begin(state.pool), end(state.pool), [](const auto& monster) { return !monster.isDefeated(); });
  const bool hasMonster = monsterIt != end(state.pool);
  const auto& hero = state.hero;
  return std::visit(
      overloaded{
          [&](Attack) { return hasMonster; },
          [&](Cast cast) {
            return hero.has(cast.spell) && ((hasMonster && Magic::isPossible(hero, *monsterIt, cast.spell)) ||
                                            (!Magic::needsMonster(cast.spell) && Magic::isPossible(hero, cast.spell)));
          },
          [&](Uncover uncover) { return state.resources.numBlackTiles >= uncover.numTiles; },
          [&, &shops = state.resources.shops](Buy buy) {
            return hero.hasRoomFor(buy.item) && hero.gold() >= price(buy.item) &&
                   std::find(begin(shops), end(shops), buy.item) != end(shops);
          },
          [&](Use use) { return hero.has(use.item) && hero.canUse(use.item); },
          [&](Convert convert) { return hero.canConvert(convert.itemOrSpell); },
          [&hero, &spells = state.resources.spells](Find find) {
            return hero.hasRoomFor(find.spell) && std::find(begin(spells), end(spells), find.spell) != end(spells);
          },
          [piety = hero.getPiety(), current = hero.getFollowedDeity(),
           &altars = state.resources.altars](Follow follow) {
            return (!current || (current != follow.deity && piety >= 50)) &&
                   std::find(begin(altars), end(altars), follow.deity) != end(altars);
          },
          [&faith = hero.getFaith(), &hero, &pactMaker = state.resources.pactMakerAvailable](Request request) {
            if (const auto boon = std::get_if<Boon>(&request.boonOrPact))
              return faith.isAvailable(*boon, hero) && hero.getPiety() >= faith.getCosts(*boon, hero);
            return pactMaker && !faith.getPact() &&
                   (!faith.enteredConsensus() || std::get<Pact>(request.boonOrPact) != Pact::Consensus);
          },
          [current = hero.getFollowedDeity(), &altars = state.resources.altars](Desecrate desecrate) {
            return current != desecrate.altar && std::find(begin(altars), end(altars), desecrate.altar) != end(altars);
          }},
      step);
}
