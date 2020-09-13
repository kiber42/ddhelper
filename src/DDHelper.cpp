// IntelliSense gets confused without the relative path
#include "../include/ImguiApp.hpp"

#include "Combat.hpp"
#include "Defence.hpp"
#include "Hero.hpp"
#include "Monster.hpp"
#include "MonsterFactory.hpp"
#include "Spells.hpp"

#include <array>
#include <optional>
#include <utility>
#include <vector>

class HeroSelection
{
public:
  HeroSelection();
  std::optional<Hero> run();
  Hero get() const;

private:
  HeroClass selectedClass;
  int selectedClassIndex;
  int level;
};

class CustomHeroBuilder
{
public:
  CustomHeroBuilder();
  std::optional<Hero> run();
  Hero get() const;

private:
  std::array<int, 8> data;
  std::map<HeroStatus, int> statuses;
};

class CustomMonsterBuilder
{
public:
  CustomMonsterBuilder();
  std::optional<Monster> run();
  Monster get() const;

private:
  std::array<int, 6> data;
  MonsterTraits traits;
};

class Arena
{
public:
  void enter(Hero&&);
  void enter(Monster&&);
  void run();

private:
  std::optional<Hero> hero;
  std::optional<Monster> monster;

  std::vector<std::pair<std::optional<Hero>, std::optional<Monster>>> history;
};

class DDHelperApp : public ImguiApp
{
public:
  DDHelperApp();
  Monster monsterFromForm();

private:
  void populateFrame() override;

  HeroSelection heroSelection;
  CustomHeroBuilder heroBuilder;
  CustomMonsterBuilder monsterBuilder;
  Arena arena;
};

namespace
{
  static ImVec4 colorSafe(0, 0.5f, 0, 1);
  static ImVec4 colorWin(0.2f, 1, 0.2f, 1);
  static ImVec4 colorDeath(1, 0, 0, 1);
  static ImVec4 colorLevelUp(0.5f, 1, 0.5f, 1);
  static ImVec4 colorNotPossible(0, 0, 0, 1);

  inline const ImVec4& summaryColor(Outcome::Summary summary)
  {
    using Summary = Outcome::Summary;
    switch (summary)
    {
    case Summary::Safe:
      return colorSafe;
    case Summary::Win:
      return colorWin;
    case Summary::Death:
      return colorDeath;
    case Summary::LevelUp:
      return colorLevelUp;
    case Summary::NotPossible:
      return colorNotPossible;
    }
  }

  void showStatus(const std::optional<Hero>& hero, const std::optional<Monster>& monster)
  {
    if (hero.has_value())
    {
      if (!hero->isDefeated())
      {
        ImGui::Text("Level %i hero has %i/%i HP, %i/%i MP, %i/%i XP", hero->getLevel(), hero->getHitPoints(),
                    hero->getHitPointsMax(), hero->getManaPoints(), hero->getManaPointsMax(), hero->getXP(),
                    hero->getXPforNextLevel());
        if (hero->hasStatus(HeroStatus::FirstStrike))
          ImGui::Text("Hero has first strike");
      }
      else
        ImGui::Text("Hero was defeated.");
    }
    if (monster.has_value())
    {
      if (!monster->isDefeated())
      {
        ImGui::Text("%s has %i/%i HP", monster->getName(), monster->getHitPoints(), monster->getHitPointsMax());
        if (monster->isBurning())
          ImGui::Text("Monster is burning (burn stack size %i)", monster->getBurnStackSize());
      }
      else
        ImGui::Text("%s was defeated.", monster->getName());
    }
  }

} // namespace

DDHelperApp::DDHelperApp()
  : ImguiApp("Desktop Dungeons Simulator")
{
}

void DDHelperApp::populateFrame()
{
  auto hero = heroSelection.run();
  if (hero.has_value())
    arena.enter(std::move(hero.value()));

  auto customHero = heroBuilder.run();
  if (customHero.has_value())
    arena.enter(std::move(customHero.value()));

  auto monsterToArena = monsterBuilder.run();
  if (monsterToArena.has_value())
    arena.enter(std::move(monsterToArena.value()));

  arena.run();
}

HeroSelection::HeroSelection()
  : selectedClass(HeroClass::Fighter)
  , selectedClassIndex(0)
  , level(1)
{
}

std::optional<Hero> HeroSelection::run()
{
  constexpr std::array allClasses = {HeroClass::Fighter, HeroClass::Berserker, HeroClass::Warlord};

  ImGui::Begin("Hero");
  if (ImGui::BeginCombo("Class", to_string(selectedClass)))
  {
    int n = 0;
    for (auto theClass : allClasses)
    {
      if (ImGui::Selectable(to_string(theClass), n == selectedClassIndex))
      {
        selectedClass = theClass;
        selectedClassIndex = n;
      }
      ++n;
    }
    ImGui::EndCombo();
  }
  ImGui::DragInt("Level", &level, 0.1f, 1, 10);

  std::optional<Hero> newHero;
  if (ImGui::Button("Send to Arena"))
    newHero.emplace(get());
  ImGui::End();
  return newHero;
}

Hero HeroSelection::get() const
{
  Hero hero(selectedClass);
  for (int i = 1; i < level; ++i)
    hero.gainLevel();
  return hero;
}

CustomHeroBuilder::CustomHeroBuilder()
  : data{1, 10, 10, 10, 10, 5, 0, 0}
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

  for (HeroStatus status :
       {HeroStatus::FirstStrike, HeroStatus::SlowStrike, HeroStatus::Reflexes, HeroStatus::MagicalAttack,
        HeroStatus::ConsecratedStrike, HeroStatus::HeavyFireball, HeroStatus::DeathProtection,
        HeroStatus::ExperienceBoost, HeroStatus::Poisoned, HeroStatus::ManaBurned})
  {
    bool current = statuses[status] > 0;
    if (ImGui::Checkbox(toString(status), &current))
      statuses[status] = current;
  }
  for (HeroStatus status : {HeroStatus::Corrosion, HeroStatus::Cursed, HeroStatus::Learning, HeroStatus::Weakened})
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
  Hero hero(HeroStats{maxHp, maxMp, damage}, Defence{physicalResistance, magicalResistance}, Experience{level});

  const int deltaHp = maxHp - data[1];
  const int deltaMp = maxMp - data[3];
  if (deltaHp > 0)
    hero.loseHitPointsOutsideOfFight(deltaHp);
  else if (deltaHp < 0)
    hero.healHitPoints(-deltaHp, true);
  else if (deltaMp > 0)
    hero.loseManaPoints(deltaMp);

  for (const auto& [status, intensity] : statuses)
    for (int i = 0; i < intensity; ++i)
      hero.addStatus(status);

  return hero;
}

CustomMonsterBuilder::CustomMonsterBuilder()
  : data{1, 6, 6, 3, 0, 0}
{
}

std::optional<Monster> CustomMonsterBuilder::run()
{
  ImGui::Begin("Monster");
  ImGui::DragInt("Level", &data[0], 0.2f, 1, 10);
  ImGui::DragInt2("HP / max", &data[1], 0.5f, 0, 300);
  ImGui::DragInt("Attack", &data[3], 0.5f, 0, 300);
  ImGui::DragInt("Physical Resistance", &data[4], 0.2f, 0, 100);
  ImGui::DragInt("Magical Resistance", &data[5], 0.2f, 0, 100);
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
  std::optional<Monster> monster;
  if (ImGui::Button("Send to Arena"))
    monster.emplace(get());
  ImGui::End();
  return monster;
}

Monster CustomMonsterBuilder::get() const
{
  Monster monster("Level " + std::to_string(data[0]) + " monster",
                  makeGenericMonsterStats(data[0] /* level */, data[2] /* max hp */, data[3] /* damage */,
                                          0 /* death protection */),
                  {data[4] /* physical resistance */, data[5] /* magical resistance */}, traits);
  if (data[1] < data[2])
    monster.takeDamage(data[2] - data[1], false);
  return monster;
}

void Arena::enter(Hero&& newHero)
{
  if (hero.has_value())
    history.emplace_back(std::move(hero), monster);
  hero.emplace(newHero);
}

void Arena::enter(Monster&& newMonster)
{
  if (hero.has_value() || monster.has_value())
    history.emplace_back(hero, std::move(monster));
  monster.emplace(newMonster);
}

void Arena::run()
{
  auto addActionButton = [&](const char* title, auto outcomeProvider) {
    if (ImGui::Button(title))
    {
      auto outcome = outcomeProvider();
      if (outcome.summary != Outcome::Summary::NotPossible)
      {
        history.emplace_back(std::move(hero), std::move(monster));
        hero = std::move(outcome.hero);
        monster = std::move(outcome.monster);
      }
    }
    else if (ImGui::IsItemHovered())
    {
      const auto outcome = outcomeProvider();
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      if (outcome.summary == Outcome::Summary::NotPossible)
        ImGui::TextUnformatted("Not possible");
      else
      {
        const auto color = summaryColor(outcome.summary);
        ImGui::TextColored(color, "%s", toString(outcome.summary));
        showStatus(outcome.hero, outcome.monster);
      }
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  };

  ImGui::Begin("Arena");
  showStatus(hero, monster);
  if (hero.has_value() && !hero->isDefeated())
  {
    const bool withMonster = monster.has_value() && !monster->isDefeated();
    if (withMonster)
    {
      addActionButton("Attack", [&] { return Combat::predictOutcome(hero.value(), monster.value()); });
      ImGui::SameLine();
      addActionButton("Attack Other", [&] { return Combat::attackOther(hero.value(), monster.value()); });
      ImGui::SameLine();
      addActionButton("Uncover Tile", [&] { return Combat::uncoverTiles(hero.value(), monster.value(), 1); });
    }
    else
    {
      if (ImGui::Button("Uncover Tile"))
      {
        Hero heroAfter = Combat::uncoverTiles(hero.value(), 1);
        history.emplace_back(std::move(hero), monster);
        hero = heroAfter;
      }
    }

    int count = 0;
    for (Spell spell :
         {Spell::Burndayraz, Spell::Apheelsik, Spell::Bludtupowa, Spell::Bysseps, Spell::Cydstepp, Spell::Endiswal,
          Spell::Getindare, Spell::Halpmeh, Spell::Lemmisi, Spell::Pisorf, Spell::Weytwut})
    {
      if (!withMonster && Cast::needsMonster(spell))
        continue;
      if (count++ % 4 != 0)
        ImGui::SameLine();
      if (withMonster)
        addActionButton(toString(spell), [&] { return Cast::predictOutcome(hero.value(), monster.value(), spell); });
      else if (ImGui::Button(toString(spell)) && Cast::isPossible(hero.value(), spell))
      {
        Hero heroAfter = Cast::untargeted(hero.value(), spell);
        history.emplace_back(std::move(hero), monster);
        hero = heroAfter;
      }
    }
  }
  if (!history.empty() && ImGui::Button("Undo"))
  {
    std::tie(hero, monster) = std::move(history.back());
    history.pop_back();
  }
  ImGui::End();
}

int main()
{
  DDHelperApp app;
  app.run();
}
