#include "MonsterPool.hpp"

#include "imgui.h"

Monsters::iterator runMonsterPool(Monsters& monsters)
{
  ImGui::Begin("Monster Pool");
  auto selected = end(monsters);
  int index = 0;
  for (auto monsterIt = begin(monsters); monsterIt != end(monsters); ++monsterIt)
  {
    // Button labels need to be unique
    std::string label = std::to_string(++index) + ") " + monsterIt->getName();
    if (monsterIt->isDefeated())
      label += " (dead)";
    else if (monsterIt->isBurning())
      label += " (burning: " + std::to_string(monsterIt->getBurnStackSize()) + ")";
    if (ImGui::Button(label.c_str()))
      selected = monsterIt;
  }
  ImGui::End();
  return selected;
}
