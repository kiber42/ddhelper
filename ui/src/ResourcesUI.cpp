#include "ResourcesUI.hpp"

#include "Utils.hpp"

#include "imgui.h"

namespace ui
{
  using namespace std::string_literals;

  Resources::Resources(int mapSize)
    : mapSize(mapSize)
  {
  }

  ActionResultUI Resources::run(const State& state)
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
    runSpawnShop(state);
    showStrings("Spells:", resources.visible.spells);
    runSpawnSpell(state);
    showStrings("Altars:", resources.visible.altars);
    runSpawnAltar(state);

    runCheat(state);

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

  void Resources::runSpawnShop(const State& state)
  {
    ImGui::SameLine();
    ImGui::SmallButton("Add##Shop");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("SpawnShopPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("SpawnShopPopup"))
    {
      int index = 0;
      struct SubMenu
      {
        std::string title;
        Item first;
        Item last;
      };
      const std::vector<SubMenu> submenus = {
          {"Basic Items", Item::BadgeOfHonour, Item::TrollHeart},
          {"Quest Items", Item::PiercingWand, Item::SoulOrb},
          {"Elite Items", Item::KegOfHealth, Item::WickedGuitar},
          {"Boss Rewards", Item::FabulousTreasure, Item::SensationStone},
          {"Blacksmith Items", Item::BearMace, Item::Sword},
      };
      for (auto submenu : submenus)
      {
        if (ImGui::BeginMenu(submenu.title.c_str()))
        {
          for (int itemIndex = static_cast<int>(submenu.first); itemIndex <= static_cast<int>(submenu.last);
               ++itemIndex)
          {
            const bool isSelected = ++index == selectedPopupItem;
            const auto item = static_cast<Item>(itemIndex);
            if (addPopupAction(
                    state, toString(item), "Spawn "s + toString(item) + " shop",
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
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Resources::runSpawnSpell(const State& state)
  {
    ImGui::SameLine();
    ImGui::SmallButton("Add##Spell");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("SpawnSpellPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("SpawnSpellPopup"))
    {
      for (int index = 0; index <= static_cast<int>(Spell::Last); ++index)
      {
        const Spell spell = static_cast<Spell>(index);
        const bool isSelected = index == selectedPopupItem;
        if (addPopupAction(
                state, toString(spell), "Spawn "s + toString(spell),
                [spell](State& state) {
                  state.resources().spells.emplace_back(spell);
                  return Summary::None;
                },
                isSelected, result))
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Resources::runSpawnAltar(const State& state)
  {
    ImGui::SameLine();
    ImGui::SmallButton("Add##Altar");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("SpawnAltarPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("SpawnAltarPopup"))
    {
      for (int index = 0; index <= static_cast<int>(God::Last) + 1; ++index)
      {
        const auto god = [index]() -> GodOrPactmaker {
          if (index <= static_cast<int>(God::Last))
            return static_cast<God>(index);
          return Pactmaker::ThePactmaker;
        }();
        const bool isSelected = index == selectedPopupItem;
        if (addPopupAction(
                state, toString(god), "Spawn "s + toString(god) + "'s altar",
                [god](State& state) {
                  state.resources().altars.emplace_back(god);
                  return Summary::None;
                },
                isSelected, result))
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Resources::runCheat(const State& state)
  {
    ImGui::Button("Cheat");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("CheatPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("CheatPopup"))
    {
      if (addPopupAction(
              state, "+50 piety", "Cheat: +50 piety",
              [](State& state) {
                state.hero.getFaith().gainPiety(50);
                return Summary::None;
              },
              0 == selectedPopupItem, result))
        selectedPopupItem = 0;
      if (addPopupAction(
              state, "+20 gold", "Cheat: +20 gold",
              [](State& state) {
                state.hero.addGold(20);
                return Summary::None;
              },
              1 == selectedPopupItem, result))
        selectedPopupItem = 1;
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }
} // namespace ui
