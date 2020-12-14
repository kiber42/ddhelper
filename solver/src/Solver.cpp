#include "Solver.hpp"

#include "Combat.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>

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
  while (!state.pool.empty() && state.pool.front().isDefeated())
    state.pool.erase(state.pool.begin());
  if (state.pool.empty())
    return state;
  auto& hero = state.hero;
  auto& monster = state.pool.front();
  std::visit(
      overloaded{
          [&](Attack) { Combat::attack(hero, monster); },
          [&](Cast cast) { Magic::cast(hero, monster, cast.spell); },
          [&](Uncover uncover) {
            hero.recover(uncover.numTiles);
            monster.recover(uncover.numTiles);
          },
          [&](Buy buy) { hero.buy(buy.item); },
          [&](Use use) { hero.use(use.item); },
          [&](Convert convert) {
            std::visit(overloaded{[&](Item item) { hero.convert(item); }, [&](Spell spell) { hero.convert(spell); }},
                       convert.itemOrSpell);
          },
          [&](Find find) { hero.receive(find.spell); },
          [&](Follow follow) { hero.followDeity(follow.deity); },
          [&](Request request) { hero.request(request.boon); },
          [&](Desecrate desecrate) { hero.desecrate(desecrate.altar); }},
      step);
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
