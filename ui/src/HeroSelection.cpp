#include "HeroSelection.hpp"

#include "imgui.h"

HeroSelection::HeroSelection()
  : selectedClass(HeroClass::Fighter)
  , selectedRace(HeroRace::Human)
  , level(1)
{
}

std::optional<Hero> HeroSelection::run()
{
  ImGui::Begin("Hero");
  ImGui::SetNextWindowSizeConstraints(ImVec2(100, 300), ImVec2(500, 1000));
  if (ImGui::BeginCombo("Class", toString(selectedClass)))
  {
    for (int n = 0; n <= static_cast<int>(HeroClass::Last); ++n)
    {
      const HeroClass theClass = static_cast<HeroClass>(n);
      if (ImGui::Selectable(toString(theClass), theClass == selectedClass))
        selectedClass = theClass;
    }
    ImGui::EndCombo();
  }

  if (!isMonsterClass(selectedClass) && ImGui::BeginCombo("Race", toString(selectedRace)))
  {
    for (int n = 0; n <= static_cast<int>(HeroRace::Last); ++n)
    {
      const HeroRace race = static_cast<HeroRace>(n);
      if (ImGui::Selectable(toString(race), race == selectedRace))
        selectedRace = race;
    }
    ImGui::EndCombo();
  }

  if (ImGui::InputInt("Level", &level, 1, 1))
    level = std::min(std::max(level, 1), 10);

  std::optional<Hero> newHero;
  if (ImGui::Button("Send to Arena"))
    newHero.emplace(get());
  ImGui::End();
  return newHero;
}

Hero HeroSelection::get() const
{
  Hero hero(selectedClass, selectedRace);
  Monsters ignore;
  for (int i = 1; i < level; ++i)
    hero.gainLevel(ignore);
  return hero;
}

CustomHeroBuilder::CustomHeroBuilder()
  : data{1, 10, 10, 10, 10, 5, 0, 0, 0}
{
}

std::optional<Hero> CustomHeroBuilder::run()
{
  ImGui::Begin("Custom Hero");
  ImGui::DragInt("Level", &data[0], 0.1f, 1, 10);
  if (ImGui::DragInt2("HP / max", &data[1], 0.5f, 0, 300))
    data[1] = std::min(data[1], data[2] * 3 / 2);
  if (ImGui::DragInt2("MP / max", &data[3], 0.1f, 0, 30))
    data[3] = std::min(data[3], data[4]);
  ImGui::DragInt("Attack", &data[5], 0.5f, 0, 300);
  ImGui::DragInt("Physical Resistance", &data[6], 0.2f, 0, 80);
  ImGui::DragInt("Magical Resistance", &data[7], 0.2f, 0, 80);
  ImGui::DragInt("XP", &data[8], 0.1f, 0, 50);

  for (HeroStatus status : {HeroStatus::FirstStrikePermanent, HeroStatus::FirstStrikeTemporary, HeroStatus::SlowStrike,
                            HeroStatus::Reflexes, HeroStatus::MagicalAttack, HeroStatus::ConsecratedStrike,
                            HeroStatus::HeavyFireball, HeroStatus::DeathProtection, HeroStatus::ExperienceBoost})
  {
    bool current = statuses[status] > 0;
    if (ImGui::Checkbox(toString(status), &current))
      statuses[status] = current;
  }
  for (HeroStatus status : {HeroStatus::Learning})
  {
    int current = statuses[status];
    if (ImGui::InputInt(toString(status), &current))
    {
      if (current < 0)
        current = 0;
      statuses[status] = current;
    }
  }
  std::optional<Hero> newHero;
  if (ImGui::Button("Send to Arena"))
    newHero.emplace(get());
  ImGui::End();
  return newHero;
}

Hero CustomHeroBuilder::get() const
{
  const int level = data[0];
  const int maxHp = data[2];
  const int maxMp = data[4];
  const int damage = data[5];
  const int physicalResistance = data[6];
  const int magicalResistance = data[7];
  const int xp = data[8];
  Hero hero(HeroStats{maxHp, maxMp, damage}, Defence{physicalResistance, magicalResistance}, Experience{level});
  Monsters ignore;
  hero.clearInventory();
  hero.gainExperienceNoBonuses(xp, ignore);

  const int deltaHp = maxHp - data[1];
  const int deltaMp = maxMp - data[3];
  if (deltaHp > 0)
    hero.loseHitPointsOutsideOfFight(deltaHp, ignore);
  else if (deltaHp < 0)
    hero.healHitPoints(-deltaHp, true);
  else if (deltaMp > 0)
    hero.loseManaPoints(deltaMp);

  for (const auto& [status, intensity] : statuses)
    for (int i = 0; i < intensity; ++i)
      hero.addStatus(status);

  return hero;
}
