#include "MonsterPool.hpp"

#include "imgui.h"

#include <algorithm>

void addMonsterToPool(Monster newMonster, Monsters& pool)
{
  // Undo functionality could result in duplicate entries
  const bool duplicate = std::find_if(cbegin(pool), cend(pool), [id = newMonster.getID()](const auto& poolMonster) {
        return poolMonster.getID() == id;
      }) != cend(pool);
  if (!duplicate)
    pool.emplace_back(std::move(newMonster));
}

std::optional<Monster> runMonsterPool(Monsters& monsters)
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
    auto monster = std::move(*selected);
    monsters.erase(selected);
    return monster;
  }
  return std::nullopt;
}
