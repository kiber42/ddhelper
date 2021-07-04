#include "ui/MonsterSelection.hpp"

#include "engine/Clamp.hpp"
#include "engine/MonsterTypes.hpp"

#include "imgui.h"

namespace ui
{
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
    ImGui::SetWindowPos(ImVec2{260, 5}, ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2{250, 150}, ImGuiCond_FirstUseEver);
    ImGui::PushItemWidth(150);
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
      auto n = 0u;
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
    ImGui::SameLine();
    if (ImGui::Button("Send to Pool"))
      poolMonster.emplace(get());
    ImGui::End();
  }

  Monster MonsterSelection::get() const
  {
    return {selectedType, clamped<uint8_t>(level, 1, 10), clampedTo<uint8_t>(dungeonMultiplier)};
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
    auto inputInt = [](auto label, auto& value, int minValue, int maxValue) {
      int valueAsInt = static_cast<int>(value);
      if (ImGui::InputInt(label, &valueAsInt))
      {
        valueAsInt = std::min(std::max(valueAsInt, minValue), maxValue);
        value = static_cast<typename std::remove_reference<decltype(value)>::type>(valueAsInt);
      }
    };

    auto inputPercent = [](auto label, auto& value) {
      int percentage = value.percent();
      if (ImGui::InputInt(label, &percentage))
      {
        percentage = std::min(std::max(percentage, 0), 100);
        value = static_cast<typename std::remove_reference<decltype(value)>::type>(percentage);
      }
    };

    auto checkbox = [&](auto label, MonsterTrait trait) {
      bool checked = has(trait);
      if (ImGui::Checkbox(label, &checked))
        toggle(trait);
    };

    ImGui::Begin("Custom Monster");
    ImGui::SetWindowPos(ImVec2{260, 160}, ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2{250, 380}, ImGuiCond_FirstUseEver);
    ImGui::PushItemWidth(80);
    inputInt("Level", data[0], 1, 10);
    ImGui::DragIntRange2("HP / max", &data[1], &data[2], 0.5f, 0, 300, "%d", nullptr, ImGuiSliderFlags_AlwaysClamp);
    inputInt("Attack", data[3], 0, 300);
    inputInt("Physical Resist", data[4], 0, 100);
    inputInt("Magical Resist", data[5], 0, 100);
    inputInt("Death Protection", data[6], 0, 50);
    inputPercent("Death Gaze %", deathGaze_);
    inputPercent("Life Steal %", lifeSteal_);
    inputPercent("Berserk at %", berserk_);
    bool rightColumn = false;
    for (MonsterTrait trait :
         {MonsterTrait::FirstStrike, MonsterTrait::MagicalAttack, MonsterTrait::Retaliate, MonsterTrait::Poisonous,
          MonsterTrait::ManaBurn, MonsterTrait::CurseBearer, MonsterTrait::Corrosive, MonsterTrait::Weakening,
          MonsterTrait::Undead, MonsterTrait::Bloodless})
    {
      if (rightColumn)
        ImGui::SameLine(120);
      rightColumn = !rightColumn;
      checkbox(toString(trait), trait);
    }
    if (ImGui::Button("Send to Arena"))
      arenaMonster.emplace(get());
    ImGui::SameLine();
    if (ImGui::Button("Send to Pool"))
      poolMonster.emplace(get());
    ImGui::End();
  }

  Monster CustomMonsterBuilder::get() const
  {
    const auto level = Level{data[0]};
    std::string name = "Level " + std::to_string(level.get()) + " monster";
    const auto hp = HitPoints{data[1]};
    const auto maxHp = HitPoints{data[2]};
    const auto damage = DamagePoints{data[3]};
    const auto deathProtection = DeathProtection{data[6]};
    auto stats = MonsterStats{level, maxHp, damage, deathProtection};
    if (hp < maxHp)
      stats.loseHitPoints(maxHp - hp);
    else if (hp > maxHp)
      stats.healHitPoints(hp - maxHp, true);
    auto defence = Defence{PhysicalResist{data[4]}, MagicalResist{data[5]}};
    return {std::move(name), std::move(stats), std::move(defence), static_cast<MonsterTraits>(*this)};
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
} // namespace ui
