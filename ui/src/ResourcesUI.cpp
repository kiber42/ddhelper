#include "ResourcesUI.hpp"

#include "Utils.hpp"

#include "imgui.h"

using namespace std::string_literals;

ResourcesUI::ResourcesUI(int mapSize)
  : mapSize(mapSize)
{
}

ActionResultUI ResourcesUI::run(const State& state)
{
  result.reset();

  auto makeEntries = [](auto makeEntry) {
    makeEntry("Wall", &ResourceSet::numWalls);
    makeEntry("Plant", &ResourceSet::numPlants);
    makeEntry("Blood Pool", &ResourceSet::numBloodPools);
    makeEntry("Health Potion", &ResourceSet::numHealthPotions);
    makeEntry("Mana Potion", &ResourceSet::numManaPotions);
    makeEntry("Gold Pile", &ResourceSet::numGoldPiles);
    makeEntry("Attack Booster", &ResourceSet::numAttackBoosters);
    makeEntry("Mana Booster", &ResourceSet::numManaBoosters);
    makeEntry("Health Booster", &ResourceSet::numHealthBoosters);
    makeEntry("Potion Shop", &ResourceSet::numPotionShops);
  };

  auto makeAddEntry = [this, &state](std::string label, int ResourceSet::*item) {
    const int count = state.resources.visible.*item;
    ImGui::Text("%2d %s%s", count, label.c_str(), count == 1 ? "" : "s");
    ImGui::SameLine();
    addActionButton(
        state, "Add", true, "Spawn " + std::move(label),
        [item](State& state) {
          ++(state.resources.visible.*item);
          return Summary::None;
        },
        result);
  };

  auto makeRevealEntry = [this, &state](std::string label, int ResourceSet::*item) {
    const int count = state.resources.hidden.*item;
    ImGui::Text("%2d %s%s", count, label.c_str(), count == 1 ? "" : "s");
    if (state.resources.numHiddenTiles > 0 && state.resources.hidden.*item > 0)
    {
      ImGui::SameLine();
      addActionButton(
          state, "Reveal", true, "Reveal " + std::move(label),
          [item](State& state) {
            auto& resources = state.resources;
            --resources.numHiddenTiles;
            ++resources.numRevealedTiles;
            --(resources.hidden.*item);
            ++(resources.visible.*item);
            return Summary::None;
          },
          result);
    }
  };

  auto showStrings = [](std::string prefix, const auto& v) {
    if (v.empty())
    {
      ImGui::TextUnformatted((prefix + " None").c_str());
      return;
    }
    std::vector<std::string> strings(v.size());
    std::transform(begin(v), end(v), begin(strings), [](auto item) { return toString(item); });
    ImGui::TextWrapped("%s", std::accumulate(begin(strings), end(strings), prefix, [](const auto& a, const auto& b) {
                               return a + " " + b;
                             }).c_str());
  };

  auto revealFromVector = [this, &state](std::string label, auto ResourceSet::*item) {
    const size_t count = (state.resources.hidden.*item).size();
    ImGui::Text("%2zu %s%s", count, label.c_str(), count == 1 ? "" : "s");
    if (state.resources.numHiddenTiles > 0 && count > 0)
    {
      ImGui::SameLine();
      addActionButton(
          state, "Reveal", true, "Reveal " + std::move(label),
          [item](State& state) {
            auto& resources = state.resources;
            --resources.numHiddenTiles;
            ++resources.numRevealedTiles;
            (resources.visible.*item).emplace_back((resources.hidden.*item).back());
            (resources.hidden.*item).pop_back();
            return Summary::None;
          },
          result);
    }
  };

  ImGui::Begin("Resources");

  const auto& resources = state.resources;
  ImGui::TextUnformatted(("Visible (" + std::to_string(resources.numRevealedTiles) + " Tiles)").c_str());
  ImGui::Separator();
  makeEntries(makeAddEntry);
  showStrings("Shops:", resources.visible.shops);
  showStrings("Spells:", resources.visible.spells);
  showStrings("Altars:", resources.visible.altars);

  runSpawnPopup(state);

  ImGui::Separator();
  ImGui::TextUnformatted(("Hidden (" + std::to_string(resources.numHiddenTiles) + " Tiles)").c_str());
  ImGui::Separator();

  makeEntries(makeRevealEntry);
  revealFromVector("Shop", &ResourceSet::shops);
  revealFromVector("Spell", &ResourceSet::spells);
  revealFromVector("Altar", &ResourceSet::altars);

  ImGui::Separator();
  ImGui::PushItemWidth(150);
  ImGui::InputInt("Map Size", &mapSize);
  addActionButton(
      state, "Reset / Randomize", false, "Reset & Randomize Resources",
      [mapSize = mapSize](State& state) {
        state.resources = state.hero.createResources({}, {}, mapSize);
        return Summary::None;
      },
      result);

  ImGui::SameLine();
  if (resources.numHiddenTiles > 0)
  {
    addActionButton(
        state, "Reveal All", false, "Reveal Full Map",
        [](State& state) {
          while (state.resources.numHiddenTiles > 0)
            state.resources.revealTile();
          return Summary::None;
        },
        result);
  }
  ImGui::End();

  return result;
}

void ResourcesUI::runSpawnPopup(const State& state)
{
  ImGui::Button("Find");
  if (ImGui::IsItemActive())
  {
    ImGui::OpenPopup("FindPopup");
    selectedPopupItem = -1;
  }
  if (ImGui::BeginPopup("FindPopup"))
  {
    int index = 0;
    if (ImGui::BeginMenu("Spells"))
    {
      for (int spellIndex = 0; spellIndex <= static_cast<int>(Spell::Last); ++spellIndex)
      {
        const Spell spell = static_cast<Spell>(spellIndex);
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction(
                state, toString(spell), "Find "s + toString(spell),
                [spell](State& state) {
                  state.hero.receive(spell);
                  return Summary::None;
                },
                isSelected, result))
          selectedPopupItem = index;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Altars"))
    {
      for (int altarIndex = 0; altarIndex <= static_cast<int>(God::Last); ++altarIndex)
      {
        const God god = static_cast<God>(altarIndex);
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction(
                state, toString(god), "Find "s + toString(god) + "'s altar",
                [god](State& state) {
                  state.resources().altars.emplace_back(god);
                  return Summary::None;
                },
                isSelected, result))
          selectedPopupItem = index;
      }
      if (!state.resources().pactmakerAvailable())
      {
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction(
                state, "The Pactmaker", "Find The Pactmaker's altar",
                [](State& state) {
                  state.resources().altars.push_back(Pactmaker::ThePactmaker);
                  return Summary::None;
                },
                isSelected, result))
          selectedPopupItem = index;
      }
      ImGui::EndMenu();
    }

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
                    state.resources().shops.emplace_back(item);
                    return Summary::None;
                  },
                  isSelected, result))
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
              ++index == selectedPopupItem, result))
        selectedPopupItem = index;
      if (addPopupAction(
              state, "+20 gold", "Cheat: +20 gold",
              [](State& state) {
                state.hero.addGold(20);
                return Summary::None;
              },
              ++index == selectedPopupItem, result))
        selectedPopupItem = index;
      ImGui::EndMenu();
    }
    if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}
