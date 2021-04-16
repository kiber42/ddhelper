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

  ImGui::TextUnformatted(("Visible (" + std::to_string(resources.numRevealedTiles) + " Tiles)").c_str());
  ImGui::Separator();
  makeEntries(makeAddEntry);
  auto showStrings = [](const auto& v, std::string prefix) {
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
  showStrings(resources.visible.shops, "Shops:");
  showStrings(resources.visible.spells, "Spells:");
  showStrings(resources.visible.altars, "Altars:");

  ImGui::Separator();
  ImGui::TextUnformatted(("Hidden (" + std::to_string(resources.numHiddenTiles) + " Tiles)").c_str());
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
  reveal(&ResourceSet::altars, "Altar");

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
