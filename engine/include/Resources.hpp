#pragma once

enum class Item;
enum class God;
enum class Spell;

#include <optional>
#include <random>
#include <set>
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

enum class ResourceModifier
{
  // Preparations
  ExtraAttackBoosters,
  ExtraManaBoosters,
  ExtraHealthBoosters,
  FlameMagnet,
  FewerGlyphs,
  ExtraGlyph,
  QuestItems, // TODO
  EliteItems, // TODO
  Apothecary,
  ExtraAltar,
  // Hero traits
  Hoarder,
  Martyr,
  Merchant,
};

static const int DefaultMapSize = 20;

struct ResourceSet
{
  ResourceSet(EmptyResources);
  ResourceSet(DefaultResources, int mapSize = DefaultMapSize);
  ResourceSet(const std::set<ResourceModifier>& modifiers, std::optional<God> preparedDeity, int mapSize = DefaultMapSize);

  void addRandomShop(std::mt19937 generator);
  void addRandomSpell(std::mt19937 generator);
  void addRandomAltar(std::mt19937 generator);

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

  bool operator==(const ResourceSet&) const = default;

private:
  void addRandomResources(int numShops, int numSpells, int numAltars);
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
  explicit SimpleResources(int mapSize = DefaultMapSize);
  explicit SimpleResources(ResourceSet visible, int mapSize = DefaultMapSize);

  ResourceSet& operator()() override { return *this; }
  const ResourceSet& operator()() const override { return *this; }

  void revealTile() override { --numHiddenTiles; }

  int mapSize;
};

// simple emulation of a map that can be revealed tile by tile
struct MapResources : public Resources
{
  explicit MapResources(int mapSize = DefaultMapSize);
  MapResources(ResourceSet visible, ResourceSet hidden, int mapSize = DefaultMapSize);
  explicit MapResources(SimpleResources resources);

  ResourceSet& operator()() override { return visible; }
  const ResourceSet& operator()() const override { return visible; }

  void revealTile() override;

  ResourceSet visible;
  ResourceSet hidden;
  std::mt19937 generator{std::random_device{}()};
};
