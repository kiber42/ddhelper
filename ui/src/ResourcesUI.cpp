#include "ResourcesUI.hpp"

#include "Utils.hpp"

#include "imgui.h"

ResourcesUI::ResourcesUI(int mapSize)
  : mapSize(mapSize)
{
}

auto ResourcesUI::run(const MapResources& resources, const Hero& hero) -> Result
{
  Result result;

  auto makeEntries = [](auto makeEntry) {
    makeEntry("Walls", &ResourceSet::numWalls);
    makeEntry("Plants", &ResourceSet::numPlants);
    makeEntry("Blood Pools", &ResourceSet::numBloodPools);
    makeEntry("Health Potions", &ResourceSet::numHealthPotions);
    makeEntry("Mana Potions", &ResourceSet::numManaPotions);
    makeEntry("Gold Piles", &ResourceSet::numGoldPiles);
    makeEntry("Attack Boosters", &ResourceSet::numAttackBoosters);
    makeEntry("Mana Boosters", &ResourceSet::numManaBoosters);
    makeEntry("Health Boosters", &ResourceSet::numHealthBoosters);
  };

  auto makeAddEntry = [&resources, &result](const char* label, int ResourceSet::*item) {
    ImGui::Text("%2d %s", resources.visible.*item, label);
    ImGui::SameLine();
    ImGui::PushID(label);
    if (ImGui::Button("Add"))
    {
      result.emplace(resources, ModificationType::ResourceAdded);
      ++(result->first.visible.*item);
    }
    ImGui::PopID();
  };

  auto makeRevealEntry = [&resources, &result](const char* label, int ResourceSet::*item) {
    ImGui::Text("%2d %s", resources.hidden.*item, label);
    ImGui::PushID(label);
    if (resources.numHiddenTiles > 0 && resources.hidden.*item > 0)
    {
      ImGui::SameLine();
      if (ImGui::Button("Reveal"))
      {
        auto updated = resources;
        --updated.numHiddenTiles;
        --(updated.hidden.*item);
        ++(updated.visible.*item);
        result.emplace(std::move(updated), ModificationType::TargetedReveal);
      }
    }
    ImGui::PopID();
  };

  ImGui::Begin("Resources");

  ImGui::TextUnformatted("Visible");
  ImGui::Separator();
  makeEntries(makeAddEntry);
  // TODO: show shops, altars?
  ImGui::Separator();

  const std::string title = "Hidden (" + std::to_string(resources.numHiddenTiles) + " Tiles)";
  ImGui::TextUnformatted(title.c_str());
  ImGui::Separator();
  if (resources.numHiddenTiles > 0 && ImGui::Button("Reveal Random Tile"))
  {
    result.emplace(resources, ModificationType::RandomReveal);
    result->first.revealTile();
  }
  if (resources.numHiddenTiles > 0 && ImGui::Button("Reveal All"))
  {
    result.emplace(resources, ModificationType::RandomReveal);
    while (result->first.numHiddenTiles > 0)
      result->first.revealTile();
  }

  makeEntries(makeRevealEntry);
  ImGui::Text("%2zu Shops", resources.hidden.shops.size());
  if (resources.numHiddenTiles > 0 && !resources.hidden.shops.empty())
  {
    ImGui::SameLine();
    if (ImGui::Button("Reveal##Shop"))
    {
      auto updated = resources;
      --updated.numHiddenTiles;
      updated.visible.shops.emplace_back(updated.hidden.shops.back());
      updated.hidden.shops.pop_back();
      result.emplace(std::move(updated), ModificationType::TargetedReveal);
    }
  }
  const size_t numAltars = resources.hidden.altars.size() + (resources.hidden.pactMakerAvailable ? 1u : 0u);
  ImGui::Text("%2zu Altars", numAltars);
  if (resources.numHiddenTiles > 0 && numAltars > 0)
  {
    ImGui::SameLine();
    if (ImGui::Button("Reveal##Altar"))
    {
      auto updated = resources;
      --updated.numHiddenTiles;
      if (resources.hidden.pactMakerAvailable &&
          std::uniform_int_distribution<>(0, numAltars - 1)(updated.generator) == 0)
      {
        updated.visible.pactMakerAvailable = true;
        updated.hidden.pactMakerAvailable = false;
      }
      else
      {
        updated.visible.altars.emplace_back(updated.hidden.altars.back());
        updated.hidden.altars.pop_back();
      }
      result.emplace(std::move(updated), ModificationType::TargetedReveal);
    }
  }
  ImGui::Separator();

  ImGui::Separator();
  ImGui::InputInt("Map Size", &mapSize);
  if (ImGui::Button("Reset / Randomize"))
    result.emplace(hero.createResources({}, mapSize), ModificationType::Reset);
  ImGui::End();

  return result;
}
