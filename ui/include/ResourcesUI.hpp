#pragma once

#include "Resources.hpp"

class ResourcesUI
{
public:
  ResourcesUI(int mapSize = 20);

  void run(MapResources& resources);

private:
  [[maybe_unused]] int mapSize;
  ResourceSet initial;
};
