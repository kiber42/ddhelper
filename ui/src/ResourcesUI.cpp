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

  auto makeAddEntry = [&resources, &result](const char* label, int ResourceSet::*item) {
    const int count = resources.visible.*item;
    ImGui::Text("%2d %s%s", count, label, count == 1 ? "" : "s");
    ImGui::SameLine();
    ImGui::PushID(label);
    if (ImGui::SmallButton("Add"))
    {
      result.emplace(resources, ModificationType::ResourceAdded);
      ++(result->first.visible.*item);
    }
    ImGui::PopID();
  };

  auto makeRevealEntry = [&resources, &result](const char* label, int ResourceSet::*item) {
    const int count = resources.hidden.*item;
    ImGui::Text("%2d %s%s", count, label, count == 1 ? "" : "s");
    ImGui::PushID(label);
    if (resources.numHiddenTiles > 0 && resources.hidden.*item > 0)
    {
      ImGui::SameLine();
      if (ImGui::SmallButton("Reveal"))
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
  auto makeStrings = [](const auto& v) {
    std::vector<std::string> strings(v.size());
    std::transform(begin(v), end(v), begin(strings), [](auto item) { return toString(item); });
    return strings;
  };
  auto showStrings = [](const auto& strings, std::string prefix) {
    if (strings.empty())
    {
      ImGui::TextUnformatted((prefix + " None").c_str());
      return;
    }
    ImGui::TextWrapped("%s", std::accumulate(begin(strings), end(strings), prefix, [](const auto& a, const auto& b) {
                               return a + " " + b;
                             }).c_str());
  };
  showStrings(makeStrings(resources.visible.shops), "Shops:");
  showStrings(makeStrings(resources.visible.spells), "Spells:");
  auto altars = makeStrings(resources.visible.altars);
  if (resources.visible.pactMakerAvailable)
    altars.push_back("The Pactmaker");
  showStrings(altars, "Altars:");

  ImGui::Separator();
  const std::string title = "Hidden (" + std::to_string(resources.numHiddenTiles) + " Tiles)";
  ImGui::TextUnformatted(title.c_str());
  ImGui::Separator();

  makeEntries(makeRevealEntry);
  auto reveal = [&resources, &result](auto ResourceSet::*item, auto label) {
    const size_t count = (resources.hidden.*item).size();
    ImGui::Text("%2zu %s%s", count, label, count == 1 ? "" : "s");
    if (resources.numHiddenTiles > 0 && count > 0)
    {
      ImGui::SameLine();
      ImGui::PushID(label);
      if (ImGui::SmallButton("Reveal"))
      {
        auto updated = resources;
        --updated.numHiddenTiles;
        (updated.visible.*item).emplace_back((updated.hidden.*item).back());
        (updated.hidden.*item).pop_back();
        result.emplace(std::move(updated), ModificationType::TargetedReveal);
      }
      ImGui::PopID();
    }
  };
  reveal(&ResourceSet::shops, "Shop");
  reveal(&ResourceSet::spells, "Spell");

  const size_t numAltars = resources.hidden.altars.size() + (resources.hidden.pactMakerAvailable ? 1u : 0u);
  ImGui::Text("%2zu Altars", numAltars);
  if (resources.numHiddenTiles > 0 && numAltars > 0)
  {
    ImGui::SameLine();
    if (ImGui::SmallButton("Reveal##Altar"))
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
  ImGui::PushItemWidth(150);
  ImGui::InputInt("Map Size", &mapSize);
  if (ImGui::Button("Reset / Randomize"))
    result.emplace(hero.createResources({}, {}, mapSize), ModificationType::Reset);
  ImGui::SameLine();
  if (resources.numHiddenTiles > 0 && ImGui::Button("Reveal All"))
  {
    result.emplace(resources, ModificationType::RandomReveal);
    while (result->first.numHiddenTiles > 0)
      result->first.revealTile();
  }
  ImGui::End();

  return result;
}
