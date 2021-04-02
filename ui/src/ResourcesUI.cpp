#include "ResourcesUI.hpp"

ResourcesUI::ResourcesUI(int mapSize)
  : mapSize(mapSize)
  , initial(initialResourceSet(mapSize))
{
}

void ResourcesUI::run(MapResources& /*resources*/) {}
