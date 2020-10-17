#include "Arena.hpp"

#include "Combat.hpp"
#include "Hero.hpp"
#include "Items.hpp"
#include "Monster.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

#include "imgui.h"

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

  inline const ImVec4& summaryColor(Summary summary, bool debuffed)
  {
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

  void showStatus(const Hero& hero)
  {
    if (!hero.isDefeated())
    {
      ImGui::Text("%s level %i has %i/%i HP, %i/%i MP, %i/%i XP, %i piety, %i damage", hero.getName().c_str(),
                  hero.getLevel(), hero.getHitPoints(), hero.getHitPointsMax(), hero.getManaPoints(),
                  hero.getManaPointsMax(), hero.getXP(), hero.getXPforNextLevel(), hero.getFaith().getPiety(),
                  hero.getDamageVersusStandard());
      if (hero.hasStatus(HeroStatus::FirstStrike))
        ImGui::Text("  has first strike");
      if (hero.hasStatus(HeroStatus::DeathProtection))
        ImGui::Text("  has death protection");
      if (hero.hasStatus(HeroStatus::Poisoned))
        ImGui::Text("  is poisoned");
      if (hero.hasStatus(HeroStatus::ManaBurned))
        ImGui::Text("  is mana burned");
      if (hero.hasStatus(HeroStatus::Cursed))
        ImGui::Text("  is cursed (x%i)", hero.getStatusIntensity(HeroStatus::Cursed));
    }
    else
      ImGui::Text("Hero defeated.");
  }

  void showStatus(const Monster& monster)
  {
    if (!monster.isDefeated())
    {
      ImGui::Text("%s has %i/%i HP and does %i damage", monster.getName(), monster.getHitPoints(),
                  monster.getHitPointsMax(), monster.getDamage());
      if (monster.getPhysicalResistPercent() > 0)
        ImGui::Text("  Physical resist %i%%", monster.getPhysicalResistPercent());
      if (monster.getMagicalResistPercent() > 0)
        ImGui::Text("  Magical resist %i%%", monster.getMagicalResistPercent());
      if (monster.isPoisonous())
        ImGui::Text("  Poisonous");
      if (monster.hasManaBurn())
        ImGui::Text("  Mana Burn");
      if (monster.bearsCurse())
        ImGui::Text("  Curse bearer");
      if (monster.getDeathGazePercent() > 0)
        ImGui::Text("  Death Gaze %i%%", monster.getDeathGazePercent());
      if (monster.getDeathProtection() > 0)
        ImGui::Text("  Death protection (x%i)", monster.getDeathProtection());
      if (monster.isBurning())
        ImGui::Text("  is burning (burn stack size %i)", monster.getBurnStackSize());
      if (monster.isPoisoned())
        ImGui::Text("  is poisoned (amount: %i)", monster.getPoisonAmount());
      if (monster.isSlowed())
        ImGui::Text("  is slowed");
    }
    else
      ImGui::Text("%s defeated.", monster.getName());
  }

  void showStatus(const State& state)
  {
    if (state.hero)
      showStatus(*state.hero);
    if (state.monster)
      showStatus(*state.monster);
  }

} // namespace

void History::add(State previous, ActionEntry entry)
{
  history.emplace_back(std::tuple(std::move(previous), std::move(entry)));
}

bool History::run()
{
  ImGui::Begin("History");
  int repeated = 1;
  for (unsigned i = 0; i < history.size(); ++i)
  {
    const auto& entry = std::get<ActionEntry>(history[i]);
    if (i < history.size() - 1)
    {
      const auto next = std::get<ActionEntry>(history[i + 1]);
      if (std::get<0>(entry) == std::get<0>(next) && std::get<2>(entry) == std::get<2>(next))
      {
        ++repeated;
        continue;
      }
    }
    ImGui::TextUnformatted(std::get<std::string>(entry).c_str());
    const auto outcome = std::get<Outcome>(entry);
    if (outcome.summary != Summary::Safe)
    {
      const auto color = summaryColor(outcome.summary, !outcome.debuffs.empty());
      ImGui::SameLine();
      ImGui::TextColored(color, "%s", toString(outcome).c_str());
    }
    if (repeated > 1)
    {
      ImGui::SameLine();
      ImGui::Text("(x%i)", repeated);
      repeated = 1;
    }
  }

  const bool undoRequested = !history.empty() && ImGui::Button("Undo");
  ImGui::End();

  return undoRequested;
}

bool History::empty() const
{
  return history.empty();
}

std::pair<State, std::optional<Monster>> History::undo()
{
  assert(!history.empty());
  std::optional<Monster> undoMonster;
  auto& restore = history.back();
  AnyAction& action = std::get<AnyAction>(std::get<ActionEntry>(restore));
  if (auto monster = std::get_if<MonsterFromPool>(&action))
    undoMonster.emplace(std::move(*monster));
  auto previousState = std::move(std::get<State>(restore));
  history.pop_back();
  return std::pair{std::move(previousState), std::move(undoMonster)};
}

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
    Outcome outcome = {summary, Combat::findDebuffs(*currentState.hero, *newState.hero), std::nullopt};
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
  auto addPopupAction = [&](std::string title, AnyAction action, bool wasSelected) {
    const bool mouseDown = ImGui::IsAnyMouseDown();
    const bool becameSelected = (ImGui::Selectable(title.c_str()) || (ImGui::IsItemHovered() && mouseDown));
    addAction(std::move(title), std::move(action), wasSelected && !mouseDown);
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
      addActionButton("Attack Other", [](Hero& hero, Monster& current) { return Combat::attackOther(hero, current); });
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
        if (!possible)
        {
          ImGui::TextColored(colorUnavailable, "%s", toString(spell));
          continue;
        }

        bool becameSelected;
        if (withMonster)
          becameSelected = addPopupAction(
              toString(spell), [spell](Hero& hero, Monster& monster) { return Cast::targeted(hero, monster, spell); },
              isSelected);
        else
          becameSelected = addPopupAction(
              toString(spell),
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
        if (addPopupAction(
                toString(item),
                [item](Hero& hero) {
                  hero.use(item);
                  return Summary::Safe;
                },
                isSelected))
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
      for (const auto& entry : currentState.hero->getItems())
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto item = std::get<Item>(entry.itemOrSpell);
        if (entry.conversionPoints >= 0)
        {
          const std::string title =
              "Convert "s + toString(item) + " (" + std::to_string(entry.conversionPoints) + " CP)";
          if (addPopupAction(
                  title,
                  [item](Hero& hero) {
                    hero.convert(item);
                    return Summary::Safe;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextUnformatted("%s", toString(item));
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
          const std::string title =
              "Convert "s + toString(spell) + " (" + std::to_string(entry.conversionPoints) + " CP)";
          if (addPopupAction(
                  title,
                  [spell](Hero& hero) {
                    hero.convert(spell);
                    return Summary::Safe;
                  },
                  isSelected))
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
    ImGui::Button("Gods");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("GodsPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("GodsPopup"))
    {
      ImGui::Text("Worship");
      ImGui::Separator();
      for (int index = 0; index <= static_cast<int>(God::Last); ++index)
      {
        const God deity = static_cast<God>(index);
        const bool isSelected = index == selectedPopupItem;
        if (addPopupAction(
                "Follow "s + toString(deity),
                [deity](Hero& hero) {
                  return hero.getFaith().followDeity(deity, hero) ? Summary::Safe : Summary::NotPossible;
                },
                isSelected))
          selectedPopupItem = index;
      }
      if (currentState.hero->getFollowedDeity())
      {
        ImGui::Separator();
        ImGui::Text("Boons");
        ImGui::Separator();
        for (int index = 0; index <= static_cast<int>(Boon::Last); ++index)
        {
          const Boon boon = static_cast<Boon>(index);
          if (deity(boon) != currentState.hero->getFollowedDeity())
            continue;
          const bool isSelected = index + 100 == selectedPopupItem;
          const int costs = currentState.hero->getBoonCosts(boon);
          if (addPopupAction(
                  "Request "s + toString(boon) + " (" + std::to_string(costs) + ")",
                  [boon](Hero& hero) {
                    return hero.getFaith().request(boon, hero) ? Summary::Safe : Summary::NotPossible;
                  },
                  isSelected))
            selectedPopupItem = index + 100;
        }
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
        if (addPopupAction(
                "Find "s + toString(potion),
                [potion](Hero& hero) {
                  hero.receive(potion);
                  return Summary::Safe;
                },
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
        if (addPopupAction(
                "Find "s + toString(spell),
                [spell](Hero& hero) {
                  hero.receive(spell);
                  return Summary::Safe;
                },
                isSelected))
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }

    const int numSquares = currentState.hero->numSquaresForFullRecovery();
    if (numSquares > 0)
    {
      ImGui::SameLine();
      if (withMonster)
      {
        addActionButton("Uncover Tile",
                        [](Hero& hero, Monster& monster) { return Combat::uncoverTiles(hero, &monster, 1); });
      }
      else
      {
        addActionButton("Uncover Tile", [](Hero& hero) { return Combat::uncoverTiles(hero, nullptr, 1); });
        if (numSquares > 1)
        {
          const std::string label = "Uncover " + std::to_string(numSquares) + " Tiles";
          ImGui::SameLine();
          addActionButton(label, [numSquares](Hero& hero) { return Combat::uncoverTiles(hero, nullptr, numSquares); });
        }
      }
    }
  }
  ImGui::End();

  return result;
}
