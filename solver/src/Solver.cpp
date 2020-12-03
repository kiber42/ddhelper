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
      if (hero.getManaPointsMax() < costs)
        return false;
      while (hero.getManaPoints() < costs && (hero.has(Item::ManaPotion) || result.extraPotions.mana != 0))
      {
        if (!hero.has(Item::ManaPotion))
          --result.extraPotions.mana;
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
      if (!fullHealth && (hero.has(Item::HealthPotion) || result.extraPotions.health != 0))
      {
        if (!hero.has(Item::HealthPotion))
          --result.extraPotions.health;
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

  IntermediateResult improve(SolverState state, IntermediateResult previousResult)
  {
    IntermediateResult result;
    const ExtraPotions initialPotions = {previousResult.extraPotions.health - 1, previousResult.extraPotions.mana + 2};
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
      result = improve(state, previousResult);
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
