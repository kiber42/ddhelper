#include "ui/RunImporter.hpp"

#include "engine/Boss.hpp"

#include "importer/GameWindow.hpp"
#include "importer/ImageCapture.hpp"
#include "importer/ImageProcessor.hpp"

#include "ui/MonsterSelection.hpp"
#include "ui/State.hpp"

#include "imgui.h"

namespace ui
{
  static constexpr std::array<const char*, 3> acquireHitPointModes = {"Never", "Always", "Smart"};

  ActionResultUI RunImporter::operator()()
  {
    ActionResultUI result;
    ImGui::Begin("Importer");
    // TODO: Update initial layout to also accomodate importer window
    //    ImGui::SetWindowPos(ImVec2{5, 545}, ImGuiCond_FirstUseEver);
    //    ImGui::SetWindowSize(ImVec2{250, 170}, ImGuiCond_FirstUseEver);
    MonsterSelection::runDungeonSelection(selectedDungeon);
    if (ImGui::BeginCombo("Acquire HP", acquireHitPointModes[acquireHitPointsMode]))
    {
      for (unsigned i = 0; i < acquireHitPointModes.size(); ++i)
      {
        if (ImGui::Selectable(acquireHitPointModes[i], acquireHitPointsMode == i))
          acquireHitPointsMode = i;
      }
      ImGui::EndCombo();
    }
    if (ImGui::Button("Run"))
    {
      importer::GameWindow gameWindow;
      if (!gameWindow.valid())
        status = "Game window not found.";
      else
      {
        importer::ImageCapture capture(gameWindow);
        importer::ImageProcessor processor(capture);
        try
        {
          processor.findMonsters(3);
        }
        catch (const std::runtime_error& e)
        {
          status = "Screenshot acquisition failed: " + std::string(e.what());
        }
        status = "Imported " + std::to_string(processor.get().monsterInfos.size()) + " monsters.";
        std::vector<Monster> monsters;
        const auto multiplier = dungeonMultiplier(selectedDungeon);
        if (acquireHitPointsMode > 0)
        {
          const bool smart = acquireHitPointsMode == 2;
          processor.extractMonsterInfos(smart);
        }
        for (const auto& monsterInfo : processor.get().monsterInfos)
        {
          auto bossType = getBossInfo(selectedDungeon, monsterInfo.type, monsterInfo.level);
          if (bossType)
          {
            std::optional hp = monsterInfo.health ? std::optional<HitPoints>{monsterInfo.health->first} : std::nullopt;
            monsters.emplace_back(create(*bossType, hp));
          }
          else
            monsters.emplace_back(monsterInfo.toMonster(multiplier));
        }
        result = {"Import State", [monsters = std::move(monsters)](State& state) {
                    state.monsterPool = monsters;
                    state.activeMonster = state.monsterPool.empty() ? std::nullopt : std::optional{0};
                    return Summary::None;
                  }};
      }
    }
    ImGui::TextUnformatted(status.c_str());
    ImGui::End();
    return result;
  }
} // namespace ui
