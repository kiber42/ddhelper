#include "ResourcesUI.hpp"

#include "Utils.hpp"

#include "imgui.h"

ResourcesUI::ResourcesUI(int mapSize)
  : mapSize(mapSize)
  , numShops(5)
  , initial(initialResourceSet(mapSize))
{
}

std::optional<std::pair<MapResources, bool>> ResourcesUI::run(const MapResources& resources)
{
  auto updatedResources = resources;
  bool changed = false;

  auto makeEntries = [](auto makeEntry) {
    makeEntry("walls", &ResourceSet::numWalls);
    makeEntry("plants", &ResourceSet::numPlants);
    makeEntry("blood pools", &ResourceSet::numBloodPools);
    makeEntry("gold piles", &ResourceSet::numGoldPiles);
  };

  auto makeAddEntry = [&resources = updatedResources.visible, &changed](const char* label, int ResourceSet::*item) {
    ImGui::Text("%2d %s", resources.*item, label);
    ImGui::SameLine();
    ImGui::PushID(label);
    if (ImGui::Button("Add"))
    {
      ++(resources.*item);
      changed = true;
    }
    ImGui::PopID();
  };

  auto makeRevealEntry = [&resources = updatedResources, &changed](const char* label, int ResourceSet::*item) {
    ImGui::Text("%2d %s", resources.hidden.*item, label);
    ImGui::SameLine();
    ImGui::PushID(label);
    if (resources.numHiddenTiles > 0)
    {
      if (resources.hidden.*item > 0)
      {
        if (ImGui::Button("Reveal"))
        {
          --resources.numHiddenTiles;
          --(resources.hidden.*item);
          ++(resources.visible.*item);
          changed = true;
        }
      }
      else
        disabledButton("Reveal");
    }
    ImGui::PopID();
  };

  ImGui::Begin("Resources");

  ImGui::TextUnformatted("Visible");
  ImGui::Separator();
  makeEntries(makeAddEntry);
  ImGui::Separator();

  const std::string title = "Hidden (" + std::to_string(resources.numHiddenTiles) + " tiles)";
  ImGui::TextUnformatted(title.c_str());
  ImGui::Separator();
  makeEntries(makeRevealEntry);
  ImGui::Separator();

  ImGui::Separator();
  ImGui::InputInt("Map size", &mapSize);
  if (ImGui::Button("Reset to initial"))
  {
    updatedResources = MapResources{{}, initialResourceSet(mapSize), mapSize};
    changed = true;
  }
  ImGui::End();
  if (changed)
    return {std::move(updatedResources)};
  else
    return std::nullopt;
}
