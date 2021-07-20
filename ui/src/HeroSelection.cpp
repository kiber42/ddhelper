#include "ui/HeroSelection.hpp"

#include "engine/Clamp.hpp"

#include "imgui.h"

namespace ui
{
  bool HeroSelection::run()
  {
    ImGui::Begin("Hero");
    ImGui::SetWindowPos(ImVec2{5, 5}, ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2{250, 150}, ImGuiCond_FirstUseEver);
    ImGui::PushItemWidth(100);
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, 300), ImVec2(500, 1000));
    if (ImGui::BeginCombo("Class", toString(setup.heroClass)))
    {
      for (int n = 0; n <= static_cast<int>(HeroClass::Last); ++n)
      {
        const HeroClass theClass = static_cast<HeroClass>(n);
        if (ImGui::Selectable(toString(theClass), theClass == setup.heroClass))
          setup.heroClass = theClass;
      }
      ImGui::EndCombo();
    }

    if (!isMonsterClass(setup.heroClass) && ImGui::BeginCombo("Race", toString(setup.heroRace)))
    {
      for (int n = 0; n <= static_cast<int>(HeroRace::Last); ++n)
      {
        const HeroRace race = static_cast<HeroRace>(n);
        if (ImGui::Selectable(toString(race), race == setup.heroRace))
          setup.heroRace = race;
      }
      ImGui::EndCombo();
    }

    if (ImGui::InputInt("Level", &level, 1, 1))
      level = std::min(std::max(level, 1), 10);

    const std::string preview = setup.altar ? toString(*setup.altar) : "None";
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, 100), ImVec2(500, 1000));
    if (ImGui::BeginCombo("Altar", preview.c_str()))
    {
      if (ImGui::Selectable("None", !setup.altar))
        setup.altar.reset();
      for (int index = 0; index <= static_cast<int>(God::Last) + 1; ++index)
      {
        const auto altar = [index]() -> GodOrPactmaker {
          if (index <= static_cast<int>(God::Last))
            return static_cast<God>(index);
          return Pactmaker::ThePactmaker;
        }();
        const bool isSelected = altar == setup.altar;
        if (ImGui::Selectable(toString(altar), isSelected))
          setup.altar = altar;
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    const bool clicked = ImGui::Button("Send to Arena");
    ImGui::End();
    return clicked;
  }

  std::pair<Hero, MapResources> HeroSelection::getHeroAndResources() const
  {
    MapResources resources{setup};
    Hero hero{setup, resources.getAllAltars()};
    // Take hero to requested level; Goatperson requires special care
    Monsters ignore;
    auto& faith = hero.getFaith();
    faith.gainPiety(100);
    for (int i = 1; i < level; ++i)
      hero.gainLevel(ignore);
    faith.losePiety(faith.getPiety(), hero, ignore);
    hero.healHitPoints(hero.getHitPointsMax());
    return {std::move(hero), std::move(resources)};
  }

  CustomHeroBuilder::CustomHeroBuilder()
    : data{1, 10, 10, 10, 10, 5, 0, 0}
  {
  }

  std::optional<Hero> CustomHeroBuilder::run()
  {
    auto inputInt = [](auto label, int& value, int minValue, int maxValue) {
      if (ImGui::InputInt(label, &value))
        value = std::min(std::max(value, minValue), maxValue);
      return value;
    };

    constexpr std::array statusesWithIntensity = {
        HeroStatus::CorrosiveStrike, HeroStatus::DamageReduction, HeroStatus::DeathGaze,
        HeroStatus::DodgePermanent,  HeroStatus::Learning,        HeroStatus::LifeSteal,
        HeroStatus::Momentum,        HeroStatus::Poisonous,       HeroStatus::Sanguine,
    };

    constexpr std::array statusesWithoutIntensity = {
        HeroStatus::FirstStrikePermanent, HeroStatus::SlowStrike,     HeroStatus::MagicalAttack,
        HeroStatus::BurningStrike,        HeroStatus::HeavyFireball,  HeroStatus::DeathProtection,
        HeroStatus::PoisonImmune,         HeroStatus::ManaBurnImmune, HeroStatus::CurseImmune,
        HeroStatus::DeathGazeImmune,
    };

    ImGui::Begin("Custom Hero");
    ImGui::SetWindowPos(ImVec2{5, 160}, ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2{250, 380}, ImGuiCond_FirstUseEver);
    ImGui::PushItemWidth(80);
    inputInt("Level", data[0], 1, 10);
    ImGui::DragIntRange2("HP / max", &data[1], &data[2], 0.5f, 0, 300, "%d", nullptr, ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragIntRange2("MP / max", &data[3], &data[4], 0.1f, 0, 30, "%d", nullptr, ImGuiSliderFlags_AlwaysClamp);
    inputInt("Attack", data[5], 0, 300);
    inputInt("Damage Bonus", data[6], -100, 300);
    inputInt("Physical Resist", data[7], 0, 100);
    inputInt("Magical Resist", data[8], 0, 100);
    for (HeroStatus status : statusesWithIntensity)
    {
      if (auto statusIter = statuses.find(status); statusIter != statuses.end())
      {
        if (inputInt(toString(status), statusIter->second, -1, 100) == -1)
          statuses.erase(statusIter);
      }
    }
    ImGui::PopItemWidth();

    for (HeroStatus status : statusesWithoutIntensity)
    {
      if (statuses.find(status) != statuses.end())
      {
        ImGui::TextUnformatted(toString(status));
        ImGui::SameLine();
        ImGui::PushID(static_cast<int>(status));
        if (ImGui::SmallButton("Remove"))
          statuses.erase(status);
        ImGui::PopID();
      }
    }
    auto addChoice = [&](const auto& status) {
      if (statuses.find(status) != statuses.end())
        return;
      const bool isSelected = comboSelection == status;
      if (ImGui::Selectable(toString(status), isSelected))
        comboSelection = status;
      // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
      if (isSelected)
        ImGui::SetItemDefaultFocus();
    };
    ImGui::PushItemWidth(160);
    ImGui::SetNextWindowSizeConstraints(ImVec2(160, 100), ImVec2(500, 1000));
    if (ImGui::BeginCombo("##statuses", "Add status"))
    {
      for (HeroStatus status : statusesWithIntensity)
        addChoice(status);
      for (HeroStatus status : statusesWithoutIntensity)
        addChoice(status);
      ImGui::EndCombo();
    }
    else if (comboSelection)
    {
      statuses[*comboSelection] = 1;
      comboSelection.reset();
    }
    ImGui::PopItemWidth();

    std::optional<Hero> newHero;
    if (ImGui::Button("Send to Arena"))
      newHero.emplace(get());
    ImGui::End();
    return newHero;
  }

  Hero CustomHeroBuilder::get() const
  {
    const auto level = clamped<uint8_t>(data[0], 1, 10);
    const auto hp = HitPoints{data[1]};
    const auto maxHp = HitPoints{data[2]};
    const auto mp = ManaPoints{data[3]};
    const auto maxMp = ManaPoints{data[4]};
    const auto damage = DamagePoints{data[5]};
    const auto damageBonus = DamageBonus{data[6]};
    const auto defence = Defence{PhysicalResist{data[7]}, MagicalResist{data[8]}};
    Hero hero(HeroStats{maxHp, maxMp, damage}, defence, Experience{level});
    hero.changeDamageBonusPercent(damageBonus.in_percent() - hero.getDamageBonusPercent());
    Monsters ignore;
    hero.clearInventory();

    if (hp < maxHp)
      hero.loseHitPointsOutsideOfFight((maxHp - hp).get(), ignore);
    else
      hero.healHitPoints((hp - maxHp).get(), true);
    if (mp < maxMp)
      hero.loseManaPoints((maxMp - mp).get());

    for (const auto& [status, intensity] : statuses)
      for (int i = 0; i < intensity; ++i)
        hero.add(status);

    return hero;
  }
} // namespace ui
