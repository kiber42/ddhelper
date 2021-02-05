#include "Arena.hpp"

#include "Utils.hpp"

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
  void showPredictedOutcomeTooltip(const State& initialState, const GameAction& stateUpdate)
  {
    const auto result = applyAction(initialState, stateUpdate, true);
    const auto& newState = result.first;
    const auto& outcome = result.second;
    if (outcome.summary == Summary::None)
      return;
    createToolTip([&] {
      if (outcome.summary == Summary::NotPossible)
        ImGui::TextUnformatted("Not possible");
      else
      {
        const auto outcomeStr = toString(outcome);
        if (!outcomeStr.empty())
          ImGui::TextColored(outcomeColor(outcome), "%s", outcomeStr.c_str());
        showStatus(newState);
      }
    });
  }
} // namespace

void Arena::addAction(const State& state, std::string title, const GameAction& action, bool activated)
{
  if (activated)
    result.emplace(std::pair{std::move(title), std::move(action)});
  else if (ImGui::IsItemHovered())
    showPredictedOutcomeTooltip(state, action);
}

void Arena::addActionButton(const State& state, std::string title, const GameAction& action)
{
  const bool buttonPressed = ImGui::Button(title.c_str());
  addAction(state, std::move(title), action, buttonPressed);
}

bool Arena::addPopupAction(
    const State& state, std::string itemLabel, std::string historyTitle, const GameAction& action, bool wasSelected)
{
  const bool mouseDown = ImGui::IsAnyMouseDown();
  const bool becameSelected = (ImGui::Selectable(itemLabel.c_str()) || (ImGui::IsItemHovered() && mouseDown));
  addAction(state, std::move(historyTitle), action, wasSelected && !mouseDown);
  return becameSelected;
}

void Arena::runAttack(const State& state)
{
  const Monster* activeMonster = state.activeMonster >= 0 ? (&state.monsterPool[state.activeMonster]) : nullptr;
  if (activeMonster && !activeMonster->isDefeated())
  {
    addActionButton(state, "Attack",
                    [](State& state) { return Combat::attack(state.hero, *state.monster(), state.monsterPool); });
  }
  else
  {
    disabledButton("Attack");
    if (ImGui::IsItemHovered())
      createToolTip([] { ImGui::TextUnformatted("Not possible"); });
  }
}

void Arena::runCastPopup(const State& state)
{
  const bool withMonster = state.monster() && !state.monster()->isDefeated();
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
    for (const auto& entry : state.hero.getSpells())
    {
      const bool isSelected = ++index == selectedPopupItem;
      const auto spell = std::get<Spell>(entry.itemOrSpell);
      const bool possible = (withMonster && Magic::isPossible(state.hero, *state.monster(), spell)) ||
                            (!withMonster && !Magic::needsMonster(spell) && Magic::isPossible(state.hero, spell));
      const int costs = Magic::spellCosts(spell, state.hero);
      const std::string label = toString(spell) + " ("s + std::to_string(costs) + " MP)";
      if (!possible)
      {
        ImGui::TextColored(colorUnavailable, "%s", label.c_str());
        continue;
      }
      const std::string historyTitle = "Cast "s + toString(spell);
      auto cast = [spell, withMonster](State& state) {
        if (withMonster)
          return Magic::cast(state.hero, *state.monster(), state.monsterPool, spell);
        Magic::cast(state.hero, spell, state.monsterPool);
        return Summary::None;
      };
      if (addPopupAction(state, label, historyTitle, cast, isSelected))
        selectedPopupItem = index;
    }
    if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

void Arena::runUseItemPopup(const State& state)
{
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
    for (const auto& entry : state.hero.getItems())
    {
      const bool isSelected = ++index == selectedPopupItem;
      const auto item = std::get<Item>(entry.itemOrSpell);
      const std::string historyTitle = toString(item);
      std::string label = historyTitle;
      if (entry.count > 1)
        label += " (x" + std::to_string(entry.count) + ")";
      if (state.hero.canUse(item))
      {
        if (addPopupAction(
                state, std::move(label), historyTitle,
                [item](State& state) {
                  state.hero.use(item, state.monsterPool);
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      else if (auto monster = state.monster(); monster && !monster->isDefeated() && state.hero.canUse(item, *monster))
      {
        if (addPopupAction(
                state, std::move(label), historyTitle,
                [item](State& state) {
                  state.hero.use(item, *state.monster(), state.monsterPool);
                  return Summary::None;
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
}

void Arena::runConvertItemPopup(const State& state)
{
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
    for (const auto& entry : state.hero.getItems())
    {
      const bool isSelected = ++index == selectedPopupItem;
      const auto item = std::get<Item>(entry.itemOrSpell);
      if (entry.conversionPoints >= 0)
      {
        const std::string label = toString(item) + " ("s + std::to_string(entry.conversionPoints) + " CP)";
        const std::string historyTitle = "Convert " + label;
        if (addPopupAction(
                state, label, historyTitle,
                [item](State& state) {
                  state.hero.convert(item, state.monsterPool);
                  return Summary::None;
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
    for (const auto& entry : state.hero.getSpells())
    {
      const bool isSelected = ++index == selectedPopupItem;
      const auto spell = std::get<Spell>(entry.itemOrSpell);
      if (entry.conversionPoints >= 0)
      {
        const std::string label = toString(spell) + " ("s + std::to_string(entry.conversionPoints) + " CP)";
        const std::string historyTitle = "Convert " + label;
        if (addPopupAction(
                state, label, historyTitle,
                [spell](State& state) {
                  state.hero.convert(spell, state.monsterPool);
                  return Summary::None;
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
}

void Arena::runShopPopup(const State& state)
{
  ImGui::Button("Buy");
  if (ImGui::IsItemActive())
  {
    ImGui::OpenPopup("ShopPopup");
    selectedPopupItem = -1;
  }
  if (ImGui::BeginPopup("ShopPopup"))
  {
    if (state.resources.shops.empty())
      ImGui::TextUnformatted("No shops");
    int index;
    bool havePotionShop = false;
    for (size_t itemIndex = 0u; itemIndex < state.resources.shops.size(); ++itemIndex)
    {
      const auto item = state.resources.shops[itemIndex];
      if (item == Item::AnyPotion)
      {
        havePotionShop = true;
        continue;
      }
      const bool isSelected = ++index == selectedPopupItem;
      const int price = state.hero.cost(item);
      const std::string label = toString(item) + " ("s + std::to_string(price) + " gold)";
      const std::string historyTitle = "Buy " + label;
      if (state.hero.gold() >= price)
      {
        if (addPopupAction(
                state, label, historyTitle,
                [item, itemIndex](State& state) {
                  if (state.hero.buy(item))
                  {
                    auto& shops = state.resources.shops;
                    shops.erase(begin(shops) + itemIndex);
                  }
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      else
        ImGui::TextColored(colorUnavailable, "%s", label.c_str());
    }
    if (havePotionShop && ImGui::BeginMenu("Potion Shop"))
    {
      for (int potionIndex = static_cast<int>(Item::HealthPotion); potionIndex <= static_cast<int>(Item::CanOfWhupaz);
           ++potionIndex)
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto potion = static_cast<Item>(potionIndex);
        const int price = state.hero.cost(potion);
        const std::string label = toString(potion) + " ("s + std::to_string(price) + " gold)";
        const std::string historyTitle = "Buy " + label;
        if (addPopupAction(
                state, label, historyTitle,
                [potion](State& state) {
                  if (state.hero.buy(potion))
                  {
                    auto& shops = state.resources.shops;
                    shops.erase(std::find(begin(shops), end(shops), Item::AnyPotion));
                  }
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      ImGui::EndMenu();
    }
    if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

void Arena::runFaithPopup(const State& state)
{
  ImGui::Button("Faith");
  if (ImGui::IsItemActive())
  {
    ImGui::OpenPopup("FaithPopup");
    selectedPopupItem = -1;
  }
  if (ImGui::BeginPopup("FaithPopup"))
  {
    if (state.hero.getFollowedDeity())
    {
      ImGui::Text("Request Boon");
      ImGui::Separator();
      for (int index = 0; index <= static_cast<int>(Boon::Last); ++index)
      {
        const Boon boon = static_cast<Boon>(index);
        if (deity(boon) != state.hero.getFollowedDeity())
          continue;
        const bool isSelected = index == selectedPopupItem;
        const int costs = state.hero.getBoonCosts(boon);
        const std::string label = toString(boon) + " ("s + std::to_string(costs) + ")";
        const std::string historyTitle = "Request "s + toString(boon);
        if (addPopupAction(
                state, label, historyTitle,
                [boon](State& state) {
                  return state.hero.getFaith().request(boon, state.hero, state.monsterPool) ? Summary::None
                                                                                            : Summary::NotPossible;
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
              state, toString(deity), "Follow "s + toString(deity),
              [deity](State& state) {
                return state.hero.getFaith().followDeity(deity, state.hero) ? Summary::None : Summary::NotPossible;
              },
              isSelected))
        selectedPopupItem = index + 100;
    }
    if (!state.hero.getFaith().getPact())
    {
      ImGui::Separator();
      ImGui::Text("Pactmaker");
      ImGui::Separator();
      const auto last = state.hero.getFaith().enteredConsensus() ? Pact::LastNoConsensus : Pact::LastWithConsensus;
      for (int index = 0; index <= static_cast<int>(last); ++index)
      {
        const Pact pact = static_cast<Pact>(index);
        const bool isSelected = index + 200 == selectedPopupItem;
        const std::string label = toString(pact);
        const std::string historyTitle = "Enter " + label;
        if (addPopupAction(
                state, label, historyTitle,
                [pact](State& state) {
                  state.hero.request(pact, state.monsterPool);
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index + 200;
      }
    }
    if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

void Arena::runUncoverTiles(const State& state)
{
  auto uncoverForAll = [](State& state, int numSquares) {
    state.hero.recover(numSquares);
    for (auto& monster : state.monsterPool)
      monster.recover(numSquares);
    return Summary::None;
  };

  addActionButton(state, "Uncover Tile", [&](State& state) { return uncoverForAll(state, 1); });

  const int numSquares = state.hero.numSquaresForFullRecovery();
  if (numSquares > 1)
  {
    ImGui::SameLine();
    const std::string label = "Uncover " + std::to_string(numSquares) + " Tiles";
    addActionButton(state, label, [&](State& state) { return uncoverForAll(state, numSquares); });
  }
}

void Arena::runFindPopup(const State& state)
{
  ImGui::Button("Find");
  if (ImGui::IsItemActive())
  {
    ImGui::OpenPopup("FindPopup");
    selectedPopupItem = -1;
  }
  if (ImGui::BeginPopup("FindPopup"))
  {
    int index;
    if (ImGui::BeginMenu("Spells"))
    {
      for (index = 0; index <= static_cast<int>(Spell::Last); ++index)
      {
        const Spell spell = static_cast<Spell>(index);
        const bool isSelected = index == selectedPopupItem;
        if (addPopupAction(
                state, toString(spell), "Find "s + toString(spell),
                [spell](State& state) {
                  state.hero.receive(spell);
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      ImGui::EndMenu();
    }

    const int isSelected = ++index == selectedPopupItem;
    if (addPopupAction(
            state, "Add potion shop", "Add potion shop",
            [](State& state) {
              state.resources.shops.emplace_back(Item::AnyPotion);
              return Summary::None;
            },
            isSelected))
      selectedPopupItem = index;

    struct SubMenu
    {
      std::string title;
      Item first;
      Item last;
    };
    const std::vector<SubMenu> submenus = {{"Blacksmith Items", Item::BearMace, Item::Sword},
                                           {"Basic Items", Item::BadgeOfHonour, Item::TrollHeart},
                                           {"Quest Items", Item::PiercingWand, Item::SoulOrb},
                                           {"Elite Items", Item::KegOfHealth, Item::WickedGuitar},
                                           {"Boss Rewards", Item::FabulousTreasure, Item::SensationStone}};
    for (auto submenu : submenus)
    {
      if (ImGui::BeginMenu(submenu.title.c_str()))
      {
        for (int itemIndex = static_cast<int>(submenu.first); itemIndex <= static_cast<int>(submenu.last); ++itemIndex)
        {
          const bool isSelected = ++index == selectedPopupItem;
          const auto item = static_cast<Item>(itemIndex);
          if (addPopupAction(
                  state, toString(item), "Add shop: "s + toString(item),
                  [item](State& state) {
                    state.resources.shops.emplace_back(item);
                    return Summary::None;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        ImGui::EndMenu();
      }
    }
    if (ImGui::BeginMenu("Cheat"))
    {
      if (addPopupAction(
              state, "+50 piety", "Cheat: +50 piety",
              [](State& state) {
                state.hero.getFaith().gainPiety(50);
                return Summary::None;
              },
              ++index == selectedPopupItem))
        selectedPopupItem = index;
      if (addPopupAction(
              state, "+20 gold", "Cheat: +20 gold",
              [](State& state) {
                state.hero.addGold(20);
                return Summary::None;
              },
              ++index == selectedPopupItem))
        selectedPopupItem = index;
      ImGui::EndMenu();
    }
    if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

Arena::ArenaResult Arena::run(const State& state)
{
  result.reset();

  ImGui::Begin("Arena");
  showStatus(state);
  if (!state.hero.isDefeated())
  {
    runAttack(state);
    ImGui::SameLine();
    runCastPopup(state);
    ImGui::SameLine();
    runUseItemPopup(state);
    ImGui::SameLine();
    runConvertItemPopup(state);

    runShopPopup(state);
    ImGui::SameLine();
    runFaithPopup(state);
    ImGui::SameLine();
    runUncoverTiles(state);

    runFindPopup(state);
  }
  ImGui::End();

  return result;
}
