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

  struct Solve1Result
  {
    bool solved{false};
    ExtraPotions extraPotions;
  };

  // Attempt to find solution that defeats one monster (extends existing solution)
  Solve1Result solve_1(Monster monster, Hero& hero, Resources& resources, Solution& solution, ExtraPotions extraPotions)
  {
    const auto initialPotions = extraPotions;
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
      if (hero.getManaPointsMax() < costs)
        return false;
      while (hero.getManaPoints() < costs && (hero.has(Item::ManaPotion) || extraPotions.mana != 0))
      {
        if (!hero.has(Item::ManaPotion))
          --extraPotions.mana;
        hero.use(Item::ManaPotion);
        solution.emplace_back(Use{Item::ManaPotion});
      }
      if (!Magic::isPossible(hero, monster, spell))
        return false;
      if (!tryGetSpell(spell))
        return false;
      Magic::cast(hero, monster, spell);
      solution.emplace_back(Cast{spell});
      return true;
    };

    while (!monster.isDefeated())
    {
      const bool fullHealth = hero.getHitPoints() >= hero.getHitPointsMax();
      Hero originalHero(hero);
      Monster originalMonster(monster);
      const Summary summary = Combat::attack(hero, monster);
      if (!hero.isDefeated())
      {
        solution.emplace_back(Attack{});
        continue;
      }

      hero = std::move(originalHero);
      monster = std::move(originalMonster);
      if (!fullHealth && (hero.has(Item::HealthPotion) || extraPotions.health != 0))
      {
        if (!hero.has(Item::HealthPotion))
          --extraPotions.health;
        hero.use(Item::HealthPotion);
        solution.emplace_back(Use{Item::HealthPotion});
        continue;
      }
      if (!hero.hasInitiativeVersus(monster) && tryCast(Spell::Getindare))
        continue;
      if (!monster.doesMagicalDamage() && !hero.hasStatus(HeroStatus::Cursed) &&
          hero.getStatusIntensity(HeroStatus::StoneSkin) < 3 && tryCast(Spell::Endiswal))
        continue;
      if (tryCast(Spell::Burndayraz))
      {
        if (hero.isDefeated())
          return {};
        continue;
      }
      return {};
    }
    assert(monster.isDefeated());
    assert(!hero.isDefeated());
    return {true, {initialPotions.health - extraPotions.health, initialPotions.mana - extraPotions.mana}};
  }

  struct Result
  {
    Solution solution;
    ExtraPotions extraPotions;
  };

  // Attempt to find solution with an infinite amount of health and mana potions available
  Result solve_initial(SolverState state)
  {
    Result result;
    while (!state.pool.empty())
    {
      const auto result1 = solve_1(std::move(state.pool.back()), state.hero, state.resources, result.solution, {-1, -1});
      if (!result1.solved)
        return {};
      result.extraPotions.health += result1.extraPotions.health;
      result.extraPotions.mana += result1.extraPotions.mana;
      state.pool.pop_back();
    }
    return result;
  }

  Result improve(SolverState state, Result previousResult)
  {
    return previousResult;
  }

  std::optional<Solution> run(SolverState state)
  {
    state.hero.addStatus(HeroStatus::Pessimist);
    std::reverse(begin(state.pool), end(state.pool));

    const auto resultInitial = solve_initial(state);
    const auto resultImproved = improve(state, resultInitial);

    return resultImproved.solution;
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
