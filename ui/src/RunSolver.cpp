#include "ui/RunSolver.hpp"

#include "engine/Combat.hpp"
#include "engine/Magic.hpp"
#include "solver/GameState.hpp"
#include "solver/Solver.hpp"

#include "imgui.h"

namespace ui
{
  GameState solverStateFromUIState(const State& state)
  {
    auto solverState = GameState{
        .hero{state.hero},
        .visibleMonsters{state.monsterPool},
        .resources{state.resources.visible},
    };
    solverState.resources.numHiddenTiles = state.resources.numHiddenTiles;
    solverState.resources.numRevealedTiles = state.resources.numRevealedTiles;
    solverState.resources.ruleset = state.resources.ruleset;
    auto& monsters = solverState.visibleMonsters;
    if (state.activeMonster && *state.activeMonster > 0)
    {
      auto moveToFront = static_cast<int64_t>(*state.activeMonster);
      std::rotate(begin(monsters), begin(monsters) + moveToFront, begin(monsters) + moveToFront + 1);
    }
    if (!monsters.empty() && monsters.front().isDefeated())
      monsters.erase(begin(monsters));
    return solverState;
  }

  void applyStepToUIState(const Step& step, State& state)
  {
    if (state.monsterPool.empty())
      return;
    if (!state.monster())
      state.activeMonster = 0u;
    if (state.monster()->isDefeated())
    {
      state.monsterPool.erase(begin(state.monsterPool) + static_cast<std::int64_t>(*state.activeMonster));
      state.activeMonster.reset();
      if (state.monsterPool.empty())
        return;
      state.activeMonster = 0u;
    }
    auto& hero = state.hero;
    auto& monster = *state.monster();
    std::visit(
        overloaded{
            [&](Attack) { Combat::attack(hero, monster, state.monsterPool, state.resources); },
            [&](Cast cast) { Magic::cast(hero, monster, cast.spell, state.monsterPool, state.resources); },
            [&](Uncover uncover) {
              hero.recover(uncover.numTiles, state.monsterPool);
              monster.recover(uncover.numTiles);
              for (auto i = 0u; i < uncover.numTiles; ++i)
                state.resources.revealTile();
            },
            [&hero, &shops = state.resources.visible.shops](Buy buy) {
              shops.erase(std::find(begin(shops), end(shops), buy.item));
              hero.buy(buy.item);
            },
            [&](Use use) { hero.use(use.item, state.monsterPool); },
            [&](Convert convert) { hero.convert(convert.itemOrSpell, state.monsterPool); },
            [&hero, &spells = state.resources.visible.spells](Find find) {
              if (hero.receive(find.spell))
                spells.erase(std::find(begin(spells), end(spells), find.spell));
            },
            [&hero, &spells = state.resources.visible.freeSpells](FindFree find) {
              if (hero.receiveFreeSpell(find.spell))
                spells.erase(std::find(begin(spells), end(spells), find.spell));
            },
            [&](Follow follow) { hero.followDeity(follow.deity, state.resources.numRevealedTiles, state.resources); },
            [&](Request request) { hero.request(request.boonOrPact, state.monsterPool, state.resources); },
            [&hero, &monsters = state.monsterPool, &altars = state.resources.visible.altars](Desecrate desecrate) {
              if (hero.desecrate(desecrate.altar, monsters))
                altars.erase(std::find(begin(altars), end(altars), GodOrPactmaker{desecrate.altar}));
            },
            [](NoOp) {}},
        step);
  }

  ActionResultUI RunSolver::operator()(const State& state)
  {
    ActionResultUI result;
    ImGui::Begin("Solver");
    ImGui::SetWindowPos(ImVec2{5, 545}, ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2{250, 170}, ImGuiCond_FirstUseEver);
    if (ImGui::SmallButton("Run"))
    {
      solverSteps = run(Solver::GeneticAlgorithm, solverStateFromUIState(state));
      solutionIndex = 0;
      noSolutionFound = !solverSteps;
    }
    if (noSolutionFound)
      ImGui::TextUnformatted("No solution found.");
    else if (solverSteps)
    {
      for (size_t i = 0; i < solverSteps->size(); ++i)
      {
        const bool isActive = static_cast<int>(i) == solutionIndex;
        const auto marker = std::string{isActive ? '>' : ' '};
        const auto step = (*solverSteps)[i];
        const auto stepLabel = toString(step);
        const auto text = marker + stepLabel;
        if (!isActive)
          ImGui::TextUnformatted(text.c_str());
        else if (ImGui::Selectable(text.c_str()))
        {
          result = std::pair{stepLabel, [step](auto& state) {
                               applyStepToUIState(step, state);
                               return Summary::None;
                             }};
          ++solutionIndex;
        }
      }
    }
    ImGui::End();
    return result;
  }
} // namespace ui
