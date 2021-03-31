#pragma once

enum class Item;
enum class God;
enum class Spell;

#include <random>
#include <vector>

struct ResourceSet
{
  std::vector<Item> shops;
  std::vector<Spell> spells;
  std::vector<God> altars;
  int numWalls;
  int numPlants;
  int numBloodPools;
  int numGoldPiles;
  bool pactMakerAvailable;
};

struct Resources : public ResourceSet
{
  Resources(int mapSize = 20)
    : numHiddenTiles(mapSize * mapSize - 9)
  {
    numWalls = mapSize * mapSize * 4 / 10;
  }

  Resources(ResourceSet rhs, int mapSize = 20)
    : ResourceSet(std::move(rhs))
    , numHiddenTiles(mapSize * mapSize - 9)
  {
  }

  int numHiddenTiles;
};

struct MapResources
{
  MapResources(ResourceSet visible, ResourceSet hidden, int mapSize = 20)
    : visible(std::move(visible))
    , hidden(std::move(hidden))
    , numHiddenTiles(mapSize * mapSize)
  {
    revealTiles(9);
  }

  explicit MapResources(ResourceSet hiddenResources, int mapSize = 20)
    : MapResources({}, std::move(hiddenResources), mapSize)
  {
  }

  ResourceSet visible;
  ResourceSet hidden;
  int numHiddenTiles;
  std::mt19937 generator;
  void revealTile();
  void revealTiles(int n);
};
