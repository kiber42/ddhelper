#include "MonsterSelection.hpp"

#include "MonsterTypes.hpp"

#include "imgui.h"

MonsterSelection::MonsterSelection()
  : selectedType(MonsterType::Bandit)
  , level(1)
  , dungeonMultiplier(100)
  , selectedDungeonIndex(1)
{
}

void MonsterSelection::run()
{
  ImGui::Begin("Monster");
  ImGui::SetNextWindowSizeConstraints(ImVec2(100, 300), ImVec2(500, 1000));
  if (ImGui::BeginCombo("Type", toString(selectedType)))
  {
    for (int n = 0; n <= static_cast<int>(MonsterType::Last); ++n)
    {
      const auto type = static_cast<MonsterType>(n);
      if (ImGui::Selectable(toString(type), type == selectedType))
        selectedType = type;
    }
    ImGui::EndCombo();
  }
  if (ImGui::InputInt("Level", &level, 1, 1))
    level = std::min(std::max(level, 1), 10);

  constexpr std::array<std::pair<const char*, int>, 21> dungeons = {
      std::make_pair("Hobbler's Hold", 80),
      {"Den of Danger", 100},
      {"Venture Cave", 100},
      {"Western Jungle", 100},
      {"Eastern Tundra", 100},
      {"Northern Desert", 100},
      {"Southern Swamp", 100},
      {"Doubledoom", 110},
      {"Grimm's Grotto", 140},
      {"Rock Garden", 100},
      {"Cursed Oasis", 115},
      {"Shifting Passages", 130},
      {"Havendale Bridge", 105},
      {"The Labyrinth", 130},
      {"Magma Mines", 130},
      {"Hexx Ruins", 100},
      {"Ick Swamp", 120},
      {"The Slime Pit", 120},
      {"Berserker Camp", 100},
      {"Creeplight Ruins", 110},
      {"Halls of Steel", 120},
  };

  ImGui::SetNextWindowSizeConstraints(ImVec2(100, 300), ImVec2(500, 1000));
  if (ImGui::BeginCombo("Dungeon", dungeons[selectedDungeonIndex].first))
  {
    int n = 0;
    for (auto dungeon : dungeons)
    {
      char buffer[30];
      sprintf(buffer, "%s (%i%%)", dungeon.first, dungeon.second);
      if (ImGui::Selectable(buffer, n == selectedDungeonIndex))
      {
        dungeonMultiplier = dungeon.second;
        selectedDungeonIndex = n;
      }
      ++n;
    }
    ImGui::EndCombo();
  }
  if (ImGui::Button("Send to Arena"))
    arenaMonster.emplace(get());
  if (ImGui::Button("Send to Pool"))
    poolMonster.emplace(get());
  ImGui::End();
}

Monster MonsterSelection::get() const
{
  return {selectedType, level, dungeonMultiplier};
}

std::optional<Monster> MonsterSelection::toArena()
{
  std::optional<Monster> monster;
  std::swap(monster, arenaMonster);
  return monster;
}

std::optional<Monster> MonsterSelection::toPool()
{
  std::optional<Monster> monster;
  std::swap(monster, poolMonster);
  return monster;
}

CustomMonsterBuilder::CustomMonsterBuilder()
  : data{1, 6, 6, 3, 0, 0, 0}
{
}

void CustomMonsterBuilder::run()
{
  ImGui::Begin("Custom Monster");
  ImGui::DragInt("Level", &data[0], 0.1f, 1, 10);
  ImGui::DragInt2("HP / max", &data[1], 0.5f, 0, 300);
  ImGui::DragInt("Attack", &data[3], 0.5f, 0, 300);
  ImGui::DragInt("Physical Resistance", &data[4], 0.2f, 0, 100);
  ImGui::DragInt("Magical Resistance", &data[5], 0.2f, 0, 100);
  ImGui::DragInt("Death Protection", &data[6], 0.1f, 1, 50);
  ImGui::Checkbox("First Strike", &traits.firstStrike);
  ImGui::Checkbox("Magical Attack", &traits.magicalDamage);
  ImGui::Checkbox("Retaliate", &traits.retaliate);
  ImGui::Checkbox("Poisonous", &traits.poisonous);
  ImGui::Checkbox("Mana Burn", &traits.manaBurn);
  ImGui::Checkbox("Cursed", &traits.curse);
  ImGui::Checkbox("Corrosive", &traits.corrosive);
  ImGui::Checkbox("Weakening", &traits.weakening);
  ImGui::Checkbox("Undead", &traits.undead);
  ImGui::Checkbox("Bloodless ", &traits.bloodless);
  if (ImGui::InputInt("Death Gaze %", &traits.deathGazePercent))
    traits.deathGazePercent = std::min(std::max(traits.deathGazePercent, 0), 100);
  if (ImGui::InputInt("Life Steal %", &traits.lifeStealPercent))
    traits.lifeStealPercent = std::min(std::max(traits.lifeStealPercent, 0), 100);
  if (ImGui::InputInt("Berserk at %", &traits.berserkPercent))
    traits.berserkPercent = std::min(std::max(traits.berserkPercent, 0), 100);
  if (ImGui::Button("Send to Arena"))
    arenaMonster.emplace(get());
  if (ImGui::Button("Send to Pool"))
    poolMonster.emplace(get());
  ImGui::End();
}

Monster CustomMonsterBuilder::get() const
{
  const int level = data[0];
  std::string name = "Level " + std::to_string(level) + " monster";
  const int hp = data[1];
  const int maxHp = data[2];
  const int damage = data[3];
  const int deathProtection = data[6];
  auto stats = MonsterStats{level, maxHp, damage, deathProtection};
  if (hp < maxHp)
    stats.loseHitPoints(maxHp - hp);
  else if (hp > maxHp)
    stats.healHitPoints(hp - maxHp, true);
  auto defence = Defence{data[4], data[5]};
  return {std::move(name), std::move(stats), std::move(defence), traits};
}

std::optional<Monster> CustomMonsterBuilder::toArena()
{
  std::optional<Monster> monster;
  std::swap(monster, arenaMonster);
  return monster;
}

std::optional<Monster> CustomMonsterBuilder::toPool()
{
  std::optional<Monster> monster;
  std::swap(monster, poolMonster);
  return monster;
}
