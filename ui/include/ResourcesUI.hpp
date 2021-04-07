#pragma once

#include "Resources.hpp"

#include <optional>

class ResourcesUI
{
public:
  ResourcesUI(int mapSize = 20);

  enum class ModificationType
  {
    ResourceAdded,
    RandomReveal,
    TargetedReveal,
    Reset,
  };

  using Result = std::optional<std::pair<MapResources, ModificationType>>;
  Result run(const MapResources& resources);

private:
  int mapSize;
  ResourceSet initial;
};

constexpr const char* toString(ResourcesUI::ModificationType modificationType)
{
  switch (modificationType)
  {
  case ResourcesUI::ModificationType::ResourceAdded:
    return "Add Resource";
  case ResourcesUI::ModificationType::RandomReveal:
    return "Reveal Tile";
  case ResourcesUI::ModificationType::TargetedReveal:
    return "Reveal Resource";
  case ResourcesUI::ModificationType::Reset:
    return "Reset & Randomize Resources";
  }
}