#include "Arena.hpp"

#include "Utils.hpp"

#include "Combat.hpp"
#include "Hero.hpp"
#include "Items.hpp"
#include "Monster.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

#include "imgui.h"

Arena::StateUpdate Arena::run(const State& currentState)
{
  using namespace std::string_literals;

  Arena::StateUpdate result;

  auto addAction = [&](std::string title, AnyAction action, bool activated) {
    if (!activated && !ImGui::IsItemHovered())
      return;
    Summary summary;
    State newState{currentState.hero, currentState.monster};
    if (auto heroAction = std::get_if<HeroAction>(&action))
      summary = (*heroAction)(*newState.hero);
    else if (auto attackAction = std::get_if<AttackAction>(&action))
      summary = (*attackAction)(*newState.hero, *newState.monster);
    Outcome outcome = {summary, Combat::findDebuffs(*currentState.hero, *newState.hero),
                       std::nullopt /* TODO: report piety change */};
    if (!activated)
    {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      if (summary == Summary::NotPossible)
        ImGui::TextUnformatted("Not possible");
      else
      {
        const auto color = summaryColor(summary, !outcome.debuffs.empty());
        ImGui::TextColored(color, "%s", toString(outcome).c_str());
        showStatus(newState);
      }
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
    else if (summary != Summary::NotPossible)
    {
      ActionEntry entry{std::move(title), std::move(action), std::move(outcome)};
      result.emplace(std::pair(std::move(entry), std::move(newState)));
    }
  };
  auto addActionButton = [&](std::string title, AnyAction action) {
    const bool buttonPressed = ImGui::Button(title.c_str());
    addAction(std::move(title), std::move(action), buttonPressed);
  };
  auto addPopupAction = [&](std::string itemLabel, std::string historyTitle, AnyAction action, bool wasSelected) {
    const bool mouseDown = ImGui::IsAnyMouseDown();
    const bool becameSelected = (ImGui::Selectable(itemLabel.c_str()) || (ImGui::IsItemHovered() && mouseDown));
    addAction(std::move(historyTitle), std::move(action), wasSelected && !mouseDown);
    return becameSelected;
  };

  ImGui::Begin("Arena");
  showStatus(currentState);
  if (currentState.hero && !currentState.hero->isDefeated())
  {
    const bool withMonster = currentState.monster && !currentState.monster->isDefeated();
    if (withMonster)
    {
      addActionButton("Attack", [](Hero& hero, Monster& monster) { return Combat::attack(hero, monster); });
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
      for (const auto& entry : currentState.hero->getSpells())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto spell = std::get<Spell>(entry.itemOrSpell);
        const bool possible =
            (withMonster && Cast::isPossible(*currentState.hero, *currentState.monster, spell)) ||
            (!withMonster && !Cast::needsMonster(spell) && Cast::isPossible(*currentState.hero, spell));
        const int costs = Cast::spellCosts(spell, *currentState.hero);
        const std::string label = toString(spell) + " ("s + std::to_string(costs) + " MP)";
        if (!possible)
        {
          ImGui::TextColored(colorUnavailable, "%s", label.c_str());
          continue;
        }
        const std::string historyTitle = "Cast "s + toString(spell);
        bool becameSelected;
        if (withMonster)
          becameSelected = addPopupAction(
              label, historyTitle,
              [spell](Hero& hero, Monster& monster) { return Cast::targeted(hero, monster, spell); }, isSelected);
        else
          becameSelected = addPopupAction(
              label, historyTitle,
              [spell](Hero& hero) {
                Cast::untargeted(hero, spell);
                return Summary::Safe;
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
      for (const auto& entry : currentState.hero->getItems())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto item = std::get<Item>(entry.itemOrSpell);
        const std::string historyTitle = toString(item);
        std::string label = historyTitle;
        if (entry.count > 1)
          label += " (x" + std::to_string(entry.count) + ")";
        if (currentState.hero->canUse(item))
        {
          if (addPopupAction(
                  std::move(label), historyTitle,
                  [item](Hero& hero) {
                    hero.use(item);
                    return Summary::Safe;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextColored(colorUnavailable, "%s", label.c_str());
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
      for (const auto& entry : currentState.hero->getItems())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto item = std::get<Item>(entry.itemOrSpell);
        if (entry.conversionPoints >= 0)
        {
          const std::string label = toString(item) + " ("s + std::to_string(entry.conversionPoints) + " CP)";
          const std::string historyTitle = "Convert " + label;
          if (addPopupAction(
                  label, historyTitle,
                  [item](Hero& hero) {
                    hero.convert(item);
                    return Summary::Safe;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextColored(colorUnavailable, "%s", toString(item));
      }
      ImGui::Separator();
      ImGui::Text("Spells");
      ImGui::Separator();
      for (const auto& entry : currentState.hero->getSpells())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto spell = std::get<Spell>(entry.itemOrSpell);
        if (entry.conversionPoints >= 0)
        {
          const std::string label = toString(spell) + " ("s + std::to_string(entry.conversionPoints) + " CP)";
          const std::string historyTitle = "Convert " + label;
          if (addPopupAction(
                  label, historyTitle,
                  [spell](Hero& hero) {
                    hero.convert(spell);
                    return Summary::Safe;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextColored(colorUnavailable, "%s", toString(spell));
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    // Second line: actions that may or may not be available in actual game

    const int numSquares = currentState.hero->numSquaresForFullRecovery();
    if (withMonster)
    {
      addActionButton("Uncover Tile",
                      [](Hero& hero, Monster& monster) { hero.recover(1); monster.recover(1); return Summary::Safe; });
      ImGui::SameLine();
    }
    else if (numSquares > 0)
    {
      addActionButton("Uncover Tile", [](Hero& hero) { hero.recover(1); return Summary::Safe; });
      ImGui::SameLine();
      if (numSquares > 1)
      {
        const std::string label = "Uncover " + std::to_string(numSquares) + " Tiles";
        addActionButton(label, [numSquares](Hero& hero) { hero.recover(numSquares); return Summary::Safe; });
        ImGui::SameLine();
      }
    }

    ImGui::Button("Find");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("FindPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("FindPopup"))
    {
      ImGui::Text("Spells");
      ImGui::Separator();
      int index;
      for (index = 0; index <= static_cast<int>(Spell::Last); ++index)
      {
        const Spell spell = static_cast<Spell>(index);
        const bool isSelected = index == selectedPopupItem;
        if (addPopupAction(
                toString(spell), "Find "s + toString(spell),
                [spell](Hero& hero) {
                  hero.receive(spell);
                  return Summary::Safe;
                },
                isSelected))
          selectedPopupItem = index;
      }
      ImGui::Separator();
      ImGui::Text("Potions");
      ImGui::Separator();
      for (auto potion :
           {Item::HealthPotion, Item::ManaPotion, Item::FortitudeTonic, Item::BurnSalve, Item::StrengthPotion,
            Item::Schadenfreude, Item::QuicksilverPotion, Item::ReflexPotion, Item::CanOfWhupaz})
      {
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction(
                toString(potion), "Find "s + toString(potion),
                [potion](Hero& hero) {
                  hero.receive(potion);
                  return Summary::Safe;
                },
                isSelected))
          selectedPopupItem = index;
      }
      ImGui::Separator();
      if (addPopupAction(
              "+50 piety", "Cheat: +50 piety",
              [](Hero& hero) {
                hero.getFaith().gainPiety(50);
                return Summary::Safe;
              },
              ++index == selectedPopupItem))
        selectedPopupItem = index;
      if (addPopupAction(
              "+20 gold", "Cheat: +20 gold",
              [](Hero& hero) {
                hero.addGold(20);
                return Summary::Safe;
              },
              ++index == selectedPopupItem))
        selectedPopupItem = index;
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Button("Gods");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("GodsPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("GodsPopup"))
    {
      if (currentState.hero->getFollowedDeity())
      {
        ImGui::Text("Request Boon");
        ImGui::Separator();
        for (int index = 0; index <= static_cast<int>(Boon::Last); ++index)
        {
          const Boon boon = static_cast<Boon>(index);
          if (deity(boon) != currentState.hero->getFollowedDeity())
            continue;
          const bool isSelected = index == selectedPopupItem;
          const int costs = currentState.hero->getBoonCosts(boon);
          const std::string label = toString(boon) + " ("s + std::to_string(costs) + ")";
          const std::string historyTitle = "Request "s + toString(boon);
          if (addPopupAction(
                  label, historyTitle,
                  [boon](Hero& hero) {
                    return hero.getFaith().request(boon, hero) ? Summary::Safe : Summary::NotPossible;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        ImGui::Separator();
      }
      ImGui::Text("Worship");
      ImGui::Separator();
      for (int index = 0; index <= static_cast<int>(God::Last); ++index)
      {
        const God deity = static_cast<God>(index);
        const bool isSelected = index + 100 == selectedPopupItem;
        if (addPopupAction(
                toString(deity), "Follow "s + toString(deity),
                [deity](Hero& hero) {
                  return hero.getFaith().followDeity(deity, hero) ? Summary::Safe : Summary::NotPossible;
                },
                isSelected))
          selectedPopupItem = index + 100;
      }
      if (!currentState.hero->getFaith().getPact())
      {
        ImGui::Separator();
        ImGui::Text("Pactmaker");
        ImGui::Separator();
        for (int index = 0; index <= static_cast<int>(Pact::Last); ++index)
        {
          const Pact pact = static_cast<Pact>(index);
          const bool isSelected = index + 200 == selectedPopupItem;
          const std::string label = toString(pact);
          const std::string historyTitle = "Enter " + label;
          if (addPopupAction(
                  label, historyTitle,
                  [pact](Hero& hero) {
                    hero.getFaith().enter(pact);
                    return Summary::Safe;
                  },
                  isSelected))
            selectedPopupItem = index + 200;
        }
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    if (withMonster)
    {
      ImGui::SameLine();
      addActionButton("Attack Other", [](Hero& hero, Monster& current) { return Combat::attackOther(hero, current); });
    }
  }
  ImGui::End();

  return result;
}
