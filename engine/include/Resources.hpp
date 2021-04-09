#pragma once

enum class Item;
enum class God;
enum class Spell;

#include <random>
#include <vector>

struct EmptyResources
{
};

struct DefaultResources
{
};

struct ThiefResources
{
};

struct ResourceSet
{
  ResourceSet(EmptyResources);
  ResourceSet(DefaultResources, int mapSize = 20);
  // TODO: Consider dungeon preparations
  ResourceSet(bool isHoarder, bool isMartyr, bool isMerchant, int mapSize = 20);

  void addRandomShop();
  void addRandomSpell();
  void addRandomAltar();

  std::vector<Item> shops;
  std::vector<Spell> spells;
  std::vector<God> altars;
  bool pactMakerAvailable{false};
  int numWalls{0};
  int numPlants{0};
  int numBloodPools{0};
  int numHealthPotions{0};
  int numManaPotions{0};
  int numPotionShops{0};
  int numAttackBoosters{0};
  int numManaBoosters{0};
  int numHealthBoosters{0};
  int numGoldPiles{0};
  std::mt19937 generator{std::random_device{}()};
};

struct Resources
{
  explicit Resources(int mapSize);
  virtual ~Resources() = default;

  virtual ResourceSet& operator()() = 0;
  virtual const ResourceSet& operator()() const = 0;

  virtual void revealTile() = 0;
  void revealTiles(int n);

  int numHiddenTiles;
};

// simplified Resources management, everything is considered visible
struct SimpleResources
  : public Resources
  , public ResourceSet
{
  explicit SimpleResources(int mapSize);
  explicit SimpleResources(ResourceSet visible, int mapSize = 20);

  ResourceSet& operator()() override { return *this; }
  const ResourceSet& operator()() const override { return *this; }

  void revealTile() override {}

  int mapSize;
};

// simple emulation of a map that can be revealed tile by tile
struct MapResources : public Resources
{
  explicit MapResources(int mapSize);
  MapResources(ResourceSet visible, ResourceSet hidden, int mapSize = 20);
  explicit MapResources(SimpleResources resources);

  ResourceSet& operator()() override { return visible; }
  const ResourceSet& operator()() const override { return visible; }

  void revealTile() override;

  ResourceSet visible;
  ResourceSet hidden;
  std::mt19937 generator{std::random_device{}()};
};
