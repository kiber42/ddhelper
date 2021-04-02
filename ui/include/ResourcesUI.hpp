#pragma once

#include "Resources.hpp"

#include <optional>

class ResourcesUI
{
public:
  ResourcesUI(int mapSize = 20);

  std::optional<MapResources> run(const MapResources& resources);

private:
  int mapSize;
  [[maybe_unused]] int numShops;
  ResourceSet initial;
};
