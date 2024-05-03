#include "ui/RunHeuristics.hpp"

#include "solver/Heuristics.hpp"

#include "imgui.h"

namespace ui
{
  void RunHeuristics::update(const State& state)
  {
    using namespace std::string_literals;
    using namespace heuristics;

    output.clear();
    const auto& hero = state.hero;
    if (hero.isDefeated())
      return;

    std::optional<CatapultResult> levelCatapultResult;
    if (const auto& monsters = state.monsterPool; !monsters.empty())
      levelCatapultResult.emplace(checkLevelCatapult(hero, monsters));

    if (const auto* monster = state.monster(); monster && !monster->isDefeated())
    {
      if (monster->isDefeated())
      {
        // Nothing to do
      }
      else if (checkOneShot(hero, *monster) == OneShotResult::VictoryFlawless)
      {
        output.emplace_back("One-shottable");
      }
      else if (checkMeleeOnly(hero, *monster))
      {
        output.emplace_back("Direct melee win possible");
      }
      else if (const auto regenFightSolution = checkRegenFight(hero, *monster); !regenFightSolution.empty())
      {
        const auto regenFightResult = toRegenFightResult(regenFightSolution);
        output.emplace_back("Regen-fight possible: "s + toString(regenFightResult));
      }
      else
      {
        Solution regenFightCatapultSolution;
        if (levelCatapultResult && levelCatapultResult != CatapultResult::None)
          regenFightCatapultSolution = checkRegenFightWithCatapult(hero, *monster);
        if (!regenFightCatapultSolution.empty())
        {
          const auto regenFightWithCatapultResult = toRegenFightResult(regenFightCatapultSolution);
          output.emplace_back("Regen-fight possible (with catapult):");
          output.emplace_back("  "s + toString(regenFightWithCatapultResult));
        }
        else
        {
          output.emplace_back("Not winnable without resource use.");
        }
      }
    }

    if (levelCatapultResult)
      output.emplace_back("Level catapult: "s + toString(*levelCatapultResult));
  }

  void RunHeuristics::show() const
  {
    ImGui::Begin("Heuristics");
    for (const auto& line : output)
      ImGui::TextUnformatted(line.c_str());
    ImGui::End();
  }
} // namespace ui
