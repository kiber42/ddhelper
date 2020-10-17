#include "MonsterPool.hpp"

#include "imgui.h"

#include <algorithm>

void MonsterPool::add(Monster newMonster)
{
  // Undo functionality could result in duplicate entries
  const bool duplicate = std::find_if(cbegin(monsters), cend(monsters), [id = newMonster.getID()](const auto& poolMonster) {
        return poolMonster.getID() == id;
      }) != cend(monsters);
  if (!duplicate)
    monsters.emplace_back(std::move(newMonster));
}

std::optional<Monster> MonsterPool::run()
{
  ImGui::Begin("Monster Pool");
  auto selected = end(monsters);
  int index = 0;
  for (auto monsterIt = begin(monsters); monsterIt != end(monsters); ++monsterIt)
  {
    // Button labels need to be unique
    if (ImGui::Button((std::to_string(++index) + ") " + monsterIt->getName()).c_str()))
      selected = monsterIt;
  }
  ImGui::End();
  if (selected != end(monsters))
  {
    auto monster = *selected;
    monsters.erase(selected);
    return monster;
  }
  return std::nullopt;
}
