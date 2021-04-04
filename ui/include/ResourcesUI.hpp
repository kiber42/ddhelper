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
    TargetedReveal
  };

  using Result = std::optional<std::pair<MapResources, ModificationType>>;
  Result run(const MapResources& resources);

private:
  int mapSize;
  [[maybe_unused]] int numShops;
  ResourceSet initial;
};
