#include "Solver.hpp"

#include "Combat.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>
#include <random>

static std::mt19937 generator(std::random_device{"/dev/urandom"}());

Step randomStep()
{
  std::uniform_int_distribution<> randomAction(0, 10);
  std::uniform_int_distribution<> randomSpell(0, static_cast<int>(Spell::Last) - 1);
  std::uniform_int_distribution<> randomShopItem(0, static_cast<int>(Item::LastShopItem) - 1);
  std::uniform_int_distribution<> randomItem(0, static_cast<int>(Item::Last) - 1);
  std::uniform_int_distribution<> randomGod(0, static_cast<int>(God::Last) - 1);
  std::uniform_int_distribution<> randomBoon(0, static_cast<int>(Boon::Last) - 1);
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
    return Desecrate{static_cast<God>(randomGod(generator))};
  }
}

namespace GeneticAlgorithm
{
  // Random initial solution
  Solution initialSolution(SolverState state)
  {
    Solution initial;
    while (!state.hero.isDefeated() && initial.size() < 100)
    {
      Step step = randomStep();
      if (isValid(step, state))
      {
        initial.emplace_back(step);
        state = apply(std::move(step), std::move(state));
      }
    }
    return initial;
  }

  int fitnessScore(const Solution& solution)
  {
    return solution.size();
  }

  std::optional<Solution> run(SolverState state)
  {
    state.hero.addStatus(HeroStatus::Pessimist);
    const int num_generations = 10;
    const int generation_size = 100;

    std::array<Solution, generation_size> population;
    std::generate(begin(population), end(population), [&state] { return initialSolution(state); });

    return *std::max_element(begin(population), end(population),
                             [](const auto& a, const auto& b) { return a.size() < b.size(); });
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
  std::visit(
      overloaded{
          [&](Attack) { Combat::attack(hero, monster); }, [&](Cast cast) { Magic::cast(hero, monster, cast.spell); },
          [&](Uncover uncover) {
            hero.recover(uncover.numTiles);
            monster.recover(uncover.numTiles);
          },
          [&hero, &shops = state.resources.shops](Buy buy) {
            shops.erase(std::find(begin(shops), end(shops), buy.item));
            hero.buy(buy.item);
          },
          [&](Use use) { hero.use(use.item); },
          [&](Convert convert) {
            std::visit(overloaded{[&](Item item) { hero.convert(item); }, [&](Spell spell) { hero.convert(spell); }},
                       convert.itemOrSpell);
          },
          [&hero, &spells = state.resources.spells](Find find) {
            spells.erase(std::find(begin(spells), end(spells), find.spell));
            hero.receive(find.spell);
          },
          [&](Follow follow) { hero.followDeity(follow.deity); }, [&](Request request) { hero.request(request.boon); },
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

SolverState print(Solution solution, SolverState state)
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
  return state;
}

bool isValid(Step step, const SolverState& state)
{
  const auto monsterIt =
      std::find_if(begin(state.pool), end(state.pool), [](const auto& monster) { return !monster.isDefeated(); });
  const bool hasMonster = monsterIt != end(state.pool);
  const auto& hero = state.hero;
  // TODO:
  // - Buy / Find: Check inventory space
  // - Convert: Check if items are convertible
  return std::visit(
      overloaded{
          [&](Attack) { return hasMonster; },
          [&](Cast cast) {
            return hero.has(cast.spell) && ((hasMonster && Magic::isPossible(hero, *monsterIt, cast.spell)) ||
                                            (!Magic::needsMonster(cast.spell) && Magic::isPossible(hero, cast.spell)));
          },
          [&](Uncover uncover) { return state.resources.numBlackTiles >= uncover.numTiles; },
          [gold = hero.gold(), &shops = state.resources.shops](Buy buy) {
            return gold >= price(buy.item) && std::find(begin(shops), end(shops), buy.item) != end(shops);
          },
          [&](Use use) { return hero.has(use.item) && hero.canUse(use.item); },
          [&](Convert convert) {
            return std::visit(overloaded{[&hero](Item item) { return hero.has(item); },
                                         [&hero](Spell spell) { return hero.has(spell); }},
                              convert.itemOrSpell);
          },
          [&spells = state.resources.spells](Find find) {
            return std::find(begin(spells), end(spells), find.spell) != end(spells);
          },
          [piety = state.hero.getPiety(), current = state.hero.getFollowedDeity(),
           &altars = state.resources.altars](Follow follow) {
            return (!current || (current != follow.deity && piety >= 50)) &&
                   std::find(begin(altars), end(altars), follow.deity) != end(altars);
          },
          [&faith = hero.getFaith(), &hero = state.hero](Request request) {
            return faith.isAvailable(request.boon, hero) && hero.getPiety() >= faith.getCosts(request.boon, hero);
          },
          [current = state.hero.getFollowedDeity(), &altars = state.resources.altars](Desecrate desecrate) {
            return current != desecrate.altar && std::find(begin(altars), end(altars), desecrate.altar) != end(altars);
          }},
      step);
}
