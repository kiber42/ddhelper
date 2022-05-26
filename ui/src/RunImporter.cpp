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

  std::pair<std::string, ActionResultUI> runImportHelper(Dungeon selectedDungeon, unsigned int acquireHitPointsMode)
  {
    importer::GameWindow gameWindow;
    if (!gameWindow.valid())
      return {"Game window not found.", {}};
    importer::ImageCapture capture(gameWindow);
    importer::ImageProcessor processor(capture);
    try
    {
      processor.findMonsters(3);
    }
    catch (const std::runtime_error& e)
    {
      return {"Screenshot acquisition failed: " + std::string(e.what()), {}};
    }
    if (acquireHitPointsMode > 0)
    {
      const bool smart = acquireHitPointsMode == 2;
      processor.extractMonsterInfos(smart);
    }
    std::vector<Monster> monsters;
    const auto multiplier = dungeonMultiplier(selectedDungeon);
    const auto& monsterInfos = processor.get().monsterInfos;
    for (const auto& monsterInfo : monsterInfos)
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
    std::string statusMessage = "Imported " + std::to_string(monsterInfos.size()) + " monsters.";
    auto action = [monsters = std::move(monsters)](State& state) {
      state.monsterPool = monsters;
      state.activeMonster = state.monsterPool.empty() ? std::nullopt : std::optional{0};
      return Summary::None;
    };
    return {std::move(statusMessage), {{"Import State", std::move(action)}}};
  }

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
      auto helper = runImportHelper(selectedDungeon, acquireHitPointsMode);
      status = std::move(helper.first);
      result = std::move(helper.second);
    }
    ImGui::TextUnformatted(status.c_str());
    ImGui::End();
    return result;
  }
} // namespace ui
