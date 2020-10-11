#include "Arena.hpp"

#include "Combat.hpp"
#include "Hero.hpp"
#include "Items.hpp"
#include "Monster.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

#include "imgui.h"

using namespace std::string_literals;

namespace
{
  static ImVec4 colorSafe(0, 0.5f, 0, 1);
  static ImVec4 colorWin(0.2f, 1, 0.2f, 1);
  static ImVec4 colorDeath(1, 0, 0, 1);
  static ImVec4 colorLevelUp(0.5f, 1, 0.5f, 1);
  static ImVec4 colorNotPossible(0, 0, 0, 1);

  static ImVec4 colorDebuffedSafe(0, 0.5f, 0.5f, 1);
  static ImVec4 colorDebuffedWin(0, 0.8f, 0.8f, 1);
  static ImVec4 colorDebuffedLevelUp(0, 1, 1, 1);

  static ImVec4 colorUnavailable(0.7f, 0.7f, 0.7f, 1);

  inline const ImVec4& summaryColor(Outcome::Summary summary, bool debuffed)
  {
    using Summary = Outcome::Summary;
    switch (summary)
    {
    case Summary::Safe:
      return debuffed ? colorDebuffedSafe : colorSafe;
    case Summary::Win:
      return debuffed ? colorDebuffedWin : colorWin;
    case Summary::Death:
    case Summary::Petrified:
      return colorDeath;
    case Summary::LevelUp:
      return debuffed ? colorDebuffedLevelUp : colorLevelUp;
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
        ImGui::Text("%s level %i has %i/%i HP, %i/%i MP, %i/%i XP, %i damage", hero->getName().c_str(),
                    hero->getLevel(), hero->getHitPoints(), hero->getHitPointsMax(), hero->getManaPoints(),
                    hero->getManaPointsMax(), hero->getXP(), hero->getXPforNextLevel(),
                    hero->getDamageVersusStandard());
        if (hero->hasStatus(HeroStatus::FirstStrike))
          ImGui::Text("  has first strike");
        if (hero->hasStatus(HeroStatus::DeathProtection))
          ImGui::Text("  has death protection");
        if (hero->hasStatus(HeroStatus::Poisoned))
          ImGui::Text("  is poisoned");
        if (hero->hasStatus(HeroStatus::ManaBurned))
          ImGui::Text("  is mana burned");
        if (hero->hasStatus(HeroStatus::Cursed))
          ImGui::Text("  is cursed (x%i)", hero->getStatusIntensity(HeroStatus::Cursed));
      }
      else
        ImGui::Text("Hero defeated.");
    }
    if (monster.has_value())
    {
      if (!monster->isDefeated())
      {
        ImGui::Text("%s has %i/%i HP and does %i damage", monster->getName(), monster->getHitPoints(),
                    monster->getHitPointsMax(), monster->getDamage());
        if (monster->getPhysicalResistPercent() > 0)
          ImGui::Text("  Physical resist %i%%", monster->getPhysicalResistPercent());
        if (monster->getMagicalResistPercent() > 0)
          ImGui::Text("  Magical resist %i%%", monster->getMagicalResistPercent());
        if (monster->isPoisonous())
          ImGui::Text("  Poisonous");
        if (monster->hasManaBurn())
          ImGui::Text("  Mana Burn");
        if (monster->bearsCurse())
          ImGui::Text("  Curse bearer");
        if (monster->getDeathGazePercent() > 0)
          ImGui::Text("  Death Gaze %i%%", monster->getDeathGazePercent());
        if (monster->getDeathProtection() > 0)
          ImGui::Text("  Death protection (x%i)", monster->getDeathProtection());
        if (monster->isBurning())
          ImGui::Text("  is burning (burn stack size %i)", monster->getBurnStackSize());
        if (monster->isPoisoned())
          ImGui::Text("  is poisoned (amount: %i)", monster->getPoisonAmount());
        if (monster->isSlowed())
          ImGui::Text("  is slowed");
      }
      else
        ImGui::Text("%s defeated.", monster->getName());
    }
  }

} // namespace

void Arena::enter(Hero&& newHero)
{
  history.emplace_back(newHero.getName() + " enters"s, Outcome::Summary::Safe, Outcome::Debuffs{}, std::move(hero),
                       monster, false);
  hero.emplace(newHero);
}

void Arena::enter(Monster&& newMonster)
{
  history.emplace_back(newMonster.getName() + " enters"s, Outcome::Summary::Safe, Outcome::Debuffs{}, hero,
                       std::move(monster), false);
  monster.emplace(newMonster);
}

std::optional<Monster> Arena::swap(Monster&& newMonster)
{
  auto oldMonster = std::move(monster);
  history.emplace_back(newMonster.getName() + " enters (pool)"s, Outcome::Summary::Safe, Outcome::Debuffs{}, hero,
                       oldMonster, true);
  monster.emplace(std::move(newMonster));
  return oldMonster;
}

std::optional<Monster> Arena::run()
{
  using Summary = Outcome::Summary;

  auto addAction = [&](std::string title, auto outcomeProvider, bool activated) {
    if (activated)
    {
      auto outcome = outcomeProvider();
      if (outcome.summary != Summary::NotPossible)
      {
        history.emplace_back(std::move(title), outcome.summary, std::move(outcome.debuffs), std::move(hero),
                             std::move(monster), false);
        hero = std::move(outcome.hero);
        monster = std::move(outcome.monster);
      }
    }
    else if (ImGui::IsItemHovered())
    {
      const auto outcome = outcomeProvider();
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      if (outcome.summary == Summary::NotPossible)
        ImGui::TextUnformatted("Not possible");
      else
      {
        const auto color = summaryColor(outcome.summary, !outcome.debuffs.empty());
        ImGui::TextColored(color, "%s", toString(outcome.summary, outcome.debuffs).c_str());
        showStatus(outcome.hero, outcome.monster);
      }
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  };
  auto addActionButton = [&](std::string title, auto outcomeProvider) {
    const bool button = ImGui::Button(title.c_str());
    addAction(std::move(title), std::move(outcomeProvider), button);
  };
  auto addPopupAction = [&](std::string title, auto outcomeProvider, bool wasSelected) {
    const bool mouseDown = ImGui::IsAnyMouseDown();
    const bool becameSelected = (ImGui::Selectable(title.c_str()) || (ImGui::IsItemHovered() && mouseDown));
    addAction(std::move(title), std::move(outcomeProvider), wasSelected && !mouseDown);
    return becameSelected;
  };
  auto makeProvider = [&](auto heroModifier) {
    return [&, heroModifier = std::move(heroModifier)]() {
      Hero heroAfter = *hero;
      heroModifier(heroAfter);
      return Outcome{Outcome::Summary::Safe, {}, std::move(heroAfter), monster};
    };
  };

  ImGui::Begin("Arena");
  showStatus(hero, monster);
  if (hero.has_value() && !hero->isDefeated())
  {
    const bool withMonster = monster.has_value() && !monster->isDefeated();
    if (withMonster)
    {
      addActionButton("Attack", [&] { return Combat::predictOutcome(*hero, *monster); });
      ImGui::SameLine();
      addActionButton("Attack Other", [&] { return Combat::attackOther(*hero, *monster); });
      ImGui::SameLine();
    }

    ImGui::Button("Cast");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("CastPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("CastPopup"))
    {
      ImGui::Text("Spells");
      ImGui::Separator();
      int index = -1;
      for (const auto& entry : hero->getSpells())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto spell = std::get<Spell>(entry.itemOrSpell);
        const bool possible = (withMonster && Cast::isPossible(*hero, *monster, spell)) ||
                              (!withMonster && !Cast::needsMonster(spell) && Cast::isPossible(*hero, spell));
        if (!possible)
        {
          ImGui::TextColored(colorUnavailable, "%s", toString(spell));
          continue;
        }

        bool becameSelected;
        if (withMonster)
          becameSelected = addPopupAction(
              toString(spell), [&] { return Cast::predictOutcome(*hero, *monster, spell); }, isSelected);
        else
          becameSelected = addPopupAction(
              toString(spell),
              [&] {
                Hero heroAfter = Cast::untargeted(*hero, spell);
                return Outcome{Outcome::Summary::Safe, {}, std::move(heroAfter), monster};
              },
              isSelected);
        if (becameSelected)
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Button("Use");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("UseItemPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("UseItemPopup"))
    {
      ImGui::Text("Items");
      ImGui::Separator();
      int index = -1;
      for (const auto& entry : hero->getItems())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto item = std::get<Item>(entry.itemOrSpell);
        if (addPopupAction(toString(item), makeProvider([item](Hero& hero) { hero.use(item); }), isSelected))
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Button("Convert");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("ConvertPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("ConvertPopup"))
    {
      ImGui::Text("Items");
      ImGui::Separator();
      int index = -1;
      for (const auto& entry : hero->getItems())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto item = std::get<Item>(entry.itemOrSpell);
        if (entry.conversionPoints >= 0)
        {
          const std::string title =
              "Convert "s + toString(item) + " (" + std::to_string(entry.conversionPoints) + " CP)";
          if (addPopupAction(title, makeProvider([item](Hero& hero) { hero.convert(item); }), isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextUnformatted("%s", toString(item));
      }
      ImGui::Separator();
      ImGui::Text("Spells");
      ImGui::Separator();
      for (const auto& entry : hero->getSpells())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto spell = std::get<Spell>(entry.itemOrSpell);
        if (entry.conversionPoints >= 0)
        {
          const std::string title =
              "Convert "s + toString(spell) + " (" + std::to_string(entry.conversionPoints) + " CP)";
          if (addPopupAction(title, makeProvider([spell](Hero& hero) { hero.convert(spell); }), isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextUnformatted("%s", toString(spell));
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Button("Potion");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("PotionPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("PotionPopup"))
    {
      int index = -1;
      for (auto potion :
           {Item::HealthPotion, Item::ManaPotion, Item::FortitudeTonic, Item::BurnSalve, Item::StrengthPotion,
            Item::Schadenfreude, Item::QuicksilverPotion, Item::ReflexPotion, Item::CanOfWhupaz})
      {
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction("Find "s + toString(potion), makeProvider([potion](Hero& hero) { hero.receive(potion); }),
                           isSelected))
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Button("Spell");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("SpellPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("SpellPopup"))
    {
      int index = -1;
      for (Spell spell :
           {Spell::Burndayraz, Spell::Apheelsik, Spell::Bludtupowa, Spell::Bysseps, Spell::Cydstepp, Spell::Endiswal,
            Spell::Getindare, Spell::Halpmeh, Spell::Lemmisi, Spell::Pisorf, Spell::Weytwut})
      {
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction("Find "s + toString(spell), makeProvider([spell](Hero& hero) { hero.receive(spell); }),
                           isSelected))
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    const int numSquares = hero->numSquaresForFullRecovery();
    if (numSquares > 0)
    {
      ImGui::SameLine();
      addActionButton("Uncover Tile", [&] { return Combat::uncoverTiles(*hero, monster, 1); });
      if (numSquares > 1 && !withMonster)
      {
        const std::string label = "Uncover " + std::to_string(numSquares) + " Tiles";
        ImGui::SameLine();
        addActionButton(label, [&] { return Combat::uncoverTiles(*hero, monster, numSquares); });
      }
    }
  }

  std::optional<Monster> undoMonster;
  if (!history.empty() && ImGui::Button("Undo"))
  {
    auto restore = history.back();
    const bool monsterToPool = std::get<5>(restore);
    hero = std::move(std::get<3>(restore));
    if (monsterToPool)
      undoMonster = std::move(monster);
    monster = std::move(std::get<4>(restore));
    history.pop_back();
  }
  ImGui::End();

  ImGui::Begin("Protocol");
  int repeated = 1;
  for (unsigned i = 0; i < history.size(); ++i)
  {
    const auto& item = history[i];
    if (i < history.size() - 1)
    {
      const auto next = history[i + 1];
      if (std::get<0>(item) == std::get<0>(next) && std::get<1>(item) == std::get<1>(next) &&
          std::get<2>(item) == std::get<2>(next))
      {
        ++repeated;
        continue;
      }
    }
    ImGui::TextUnformatted(std::get<0>(item).c_str());
    const auto summary = std::get<1>(item);
    if (summary != Summary::Safe)
    {
      const auto debuffs = std::get<2>(item);
      const auto color = summaryColor(summary, !debuffs.empty());
      ImGui::SameLine();
      ImGui::TextColored(color, "%s", toString(summary, debuffs).c_str());
    }
    if (repeated > 1)
    {
      ImGui::SameLine();
      ImGui::Text("(x%i)", repeated);
      repeated = 1;
    }
  }
  ImGui::End();

  return undoMonster;
}
