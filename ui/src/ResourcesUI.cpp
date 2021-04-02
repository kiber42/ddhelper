#include "ResourcesUI.hpp"

#include "imgui.h"

ResourcesUI::ResourcesUI(int mapSize)
  : mapSize(mapSize)
  , numShops(5)
  , initial(initialResourceSet(mapSize))
{
}

namespace
{
  bool resourceEditor(std::string title, ResourceSet& resources)
  {
    bool changed = false;
    ImGui::TextUnformatted(title.c_str());
    ImGui::PushID(title.c_str());
    ImGui::Separator();
    changed |= ImGui::InputInt("Walls", &resources.numWalls);
    changed |= ImGui::InputInt("Plants", &resources.numPlants);
    changed |= ImGui::InputInt("Blood Pools", &resources.numBloodPools);
    changed |= ImGui::InputInt("Gold piles", &resources.numGoldPiles);
    ImGui::Separator();
    ImGui::PopID();
    return changed;
    //  std::vector<Item> shops;
    //  std::vector<Spell> spells;
    //  std::vector<God> altars;
    //  bool pactMakerAvailable;
  }
} // namespace

std::optional<MapResources> ResourcesUI::run(const MapResources& resources)
{
  auto updatedResources = resources;
  bool changed = false;
  ImGui::Begin("Resources");
  changed |= resourceEditor("Visible", updatedResources.visible);
  std::string title = "Hidden (" + std::to_string(resources.numHiddenTiles) + " tiles)";
  changed |= resourceEditor(title.c_str(), updatedResources.hidden);

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
