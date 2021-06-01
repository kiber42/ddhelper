#include "ui/MonsterPool.hpp"

#include "ui/Utils.hpp"

#include "imgui.h"

namespace ui
{
  Monsters::iterator runMonsterPool(Monsters& monsters, int activeIndex)
  {
    ImGui::Begin("Monster Pool");
    ImGui::SetWindowPos(ImVec2{515, 410}, ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2{245, 305}, ImGuiCond_FirstUseEver);
    auto selected = end(monsters);
    int index = 0;
    for (auto monsterIt = begin(monsters); monsterIt != end(monsters); ++monsterIt)
    {
      // Button labels need to be unique
      std::string label = std::to_string(++index) + ") " + monsterIt->getName();
      if (index == activeIndex + 1)
      {
        label += " (in Arena)";
        disabledButton(label.c_str());
        continue;
      }
      else if (monsterIt->isDefeated())
        label += " (dead)";
      else if (monsterIt->isBurning())
        label += " (burning: " + std::to_string(monsterIt->getBurnStackSize()) + ")";
      if (ImGui::Button(label.c_str()))
        selected = monsterIt;
      if (ImGui::IsItemHovered())
        createToolTip([=] { showStatus(*monsterIt); });
    }
    ImGui::End();
    return selected;
  }
} // namespace ui
