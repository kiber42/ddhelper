#pragma once

#include "engine/DungeonSetup.hpp"
#include "engine/GodsAndBoons.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

#include <optional>
#include <random>
#include <set>
#include <vector>

struct ResourceSet
{
  ResourceSet() = default;
  explicit ResourceSet(const DungeonSetup&);

  bool pactmakerAvailable() const;

  void addRandomShop(std::mt19937 generator);
  void addRandomSpell(std::mt19937 generator);
  void addRandomAltar(std::mt19937 generator);

  std::vector<Item> shops;
  std::vector<Spell> spells;
  std::vector<GodOrPactmaker> altars;
  std::vector<Item> onGround;
  std::vector<Spell> freeSpells;
  unsigned numWalls{0};
  unsigned numPlants{0};
  unsigned numBloodPools{0};
  unsigned numHealthPotions{0};
  unsigned numManaPotions{0};
  unsigned numPotionShops{0};
  unsigned numAttackBoosters{0};
  unsigned numManaBoosters{0};
  unsigned numHealthBoosters{0};
  unsigned numGoldPiles{0};

  bool operator==(const ResourceSet&) const = default;

private:
  void addRandomResources(int numShops, int numSpells, int numAltars);
};

enum class Ruleset
{
  Default,
  MonsterMachine1
};

struct Resources
{
  explicit Resources(unsigned char mapSize);
  virtual ~Resources() = default;

  virtual ResourceSet& operator()() = 0;
  virtual const ResourceSet& operator()() const = 0;

  virtual void revealTile() = 0;
  void revealTiles(unsigned n);

  bool uses(Ruleset) const;

  unsigned numHiddenTiles{0};
  unsigned numRevealedTiles{0};
  Ruleset ruleset{Ruleset::Default};
};

// simplified Resources management, everything is considered visible
struct SimpleResources
  : public Resources
  , public ResourceSet
{
  explicit SimpleResources(ResourceSet visible = {}, unsigned char mapSize = DefaultMapSize);

  ResourceSet& operator()() override { return *this; }
  const ResourceSet& operator()() const override { return *this; }

  void revealTile() override
  {
    if (numHiddenTiles > 0)
    {
      --numHiddenTiles;
      ++numRevealedTiles;
    }
  }

  unsigned char mapSize;
};

struct InitiallyRevealed
{
};

// simple emulation of a map that can be revealed tile by tile
struct MapResources : public Resources
{
  MapResources(ResourceSet visible, ResourceSet hidden, unsigned char mapSize = DefaultMapSize);
  explicit MapResources(unsigned char mapSize = DefaultMapSize);
  explicit MapResources(const DungeonSetup& dungeonSetup);
  explicit MapResources(SimpleResources resources);
  MapResources(SimpleResources resources, InitiallyRevealed);

  ResourceSet& operator()() override { return visible; }
  const ResourceSet& operator()() const override { return visible; }

  std::vector<God> getAllAltars() const;

  void revealTile() override;

  ResourceSet visible;
  ResourceSet hidden;
  std::mt19937 generator{std::random_device{}()};
};
