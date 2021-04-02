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

inline ResourceSet initialResourceSet(int mapSize = 20)
{
  return ResourceSet{{}, {}, {}, mapSize * mapSize * 4 / 10, 0, 0, 10, false};
}

struct Resources
{
  int numHiddenTiles;

  explicit Resources(int mapSize)
    : numHiddenTiles(mapSize * mapSize)
  {
  }
  virtual ~Resources() = default;

  virtual ResourceSet& operator()() = 0;
  virtual const ResourceSet& operator()() const = 0;

  virtual void revealTile() = 0;

  void revealTiles(int n)
  {
    for (int i = 0; i < n; ++i)
    {
      if (numHiddenTiles <= 0)
        break;
      revealTile();
    }
  }
};

// simplified Resources management, everything is considered visible
struct SimpleResources
  : public Resources
  , public ResourceSet
{
  int mapSize;

  explicit SimpleResources(int mapSize)
    : Resources(mapSize)
    , ResourceSet(initialResourceSet(mapSize))
    , mapSize(mapSize)
  {
    numHiddenTiles -= 9;
  }

  SimpleResources()
    : SimpleResources(20)
  {
  }

  explicit SimpleResources(ResourceSet visible, int mapSize = 20)
    : Resources(mapSize)
    , ResourceSet(std::move(visible))
    , mapSize(mapSize)
  {
    numHiddenTiles -= 9;
  }

  ResourceSet& operator()() override { return *this; }
  const ResourceSet& operator()() const override { return *this; }

  void revealTile() override {}
};

// simple emulation of a map that can be revealed tile by tile
struct MapResources : public Resources
{
  explicit MapResources(int mapSize)
    : Resources{mapSize}
    , visible()
    , hidden(initialResourceSet(mapSize))
  {
    revealTiles(9);
  }

  MapResources()
    : MapResources(20)
  {
  }

  MapResources(ResourceSet visible, ResourceSet hidden, int mapSize = 20)
    : Resources(mapSize)
    , visible(std::move(visible))
    , hidden(std::move(hidden))
  {
    revealTiles(9);
  }

  explicit MapResources(SimpleResources resources)
    : Resources(resources)
    , visible()
    , hidden(std::move(resources))
  {
  }

  ResourceSet visible;
  ResourceSet hidden;
  std::mt19937 generator{std::random_device{}()};

  ResourceSet& operator()() override { return visible; }
  const ResourceSet& operator()() const override { return visible; }

  void revealTile() override;
};
