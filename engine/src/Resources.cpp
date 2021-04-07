#include "Resources.hpp"

#include "Faith.hpp"
#include "Items.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>

ResourceSet::ResourceSet(EmptyResources){};

ResourceSet::ResourceSet(DefaultResources, int mapSize)
  : numWalls{mapSize * mapSize * 4 / 10}
  , numHealthPotions{3}
  , numManaPotions{3}
  , numPotionShops{1}
  , numAttackBoosters{3}
  , numManaBoosters{3}
  , numHealthBoosters{3}
  , numGoldPiles{10}
{
  for (int i = 0; i < 8; ++i)
    addRandomShop();
  for (int i = 0; i < 5; ++i)
    addRandomSpell();
  for (int i = 0; i < 3; ++i)
    addRandomAltar();
}

ResourceSet::ResourceSet(ThiefResources, int mapSize)
  : numWalls{mapSize * mapSize * 4 / 10}
  , numHealthPotions{4}
  , numManaPotions{4}
  , numPotionShops{1}
  , numAttackBoosters{4}
  , numManaBoosters{4}
  , numHealthBoosters{4}
  , numGoldPiles{13}
{
  for (int i = 0; i < 10; ++i)
    addRandomShop();
  for (int i = 0; i < 5; ++i)
    addRandomSpell();
  for (int i = 0; i < 3; ++i)
    addRandomAltar();
}

void ResourceSet::addRandomShop()
{
  Item item;
  const int n = static_cast<int>(Item::LastShopItem);
  do
  {
    item = static_cast<Item>(std::uniform_int_distribution<>(0, n)(generator));
  } while (std::find(begin(shops), end(shops), item) != end(shops) && shops.size() <= n);
  shops.emplace_back(item);
}

void ResourceSet::addRandomSpell()
{
  Spell spell;
  const int n = static_cast<int>(Spell::Last);
  do
  {
    spell = static_cast<Spell>(std::uniform_int_distribution<>(0, n)(generator));
  } while (std::find(begin(spells), end(spells), spell) != end(spells) && spells.size() <= n);
  spells.emplace_back(spell);
}

void ResourceSet::addRandomAltar()
{
  God god;
  const int n = static_cast<int>(God::Last) + 1;
  do
  {
    const int value = std::uniform_int_distribution<>(0, n)(generator);
    if (value == n && !pactMakerAvailable)
    {
      pactMakerAvailable = true;
      return;
    }
    god = static_cast<God>(value);
  } while (std::find(begin(altars), end(altars), god) != end(altars) && altars.size() < n);
  altars.emplace_back(god);
}

Resources::Resources(int mapSize)
  : numHiddenTiles(mapSize * mapSize)
{
}

void Resources::revealTiles(int n)
{
  for (int i = 0; i < n; ++i)
  {
    if (numHiddenTiles <= 0)
      break;
    revealTile();
  }
}

SimpleResources::SimpleResources(int mapSize)
  : Resources(mapSize)
  , ResourceSet(DefaultResources{}, mapSize)
  , mapSize(mapSize)
{
  numHiddenTiles -= 9;
}

SimpleResources::SimpleResources(ResourceSet visible, int mapSize)
  : Resources(mapSize)
  , ResourceSet(std::move(visible))
  , mapSize(mapSize)
{
  numHiddenTiles -= 9;
}

MapResources::MapResources(int mapSize)
  : Resources{mapSize}
  , visible(EmptyResources{})
  , hidden(DefaultResources{}, mapSize)
{
  revealTiles(9);
}

MapResources::MapResources(ResourceSet visible, ResourceSet hidden, int mapSize)
  : Resources(mapSize)
  , visible(std::move(visible))
  , hidden(std::move(hidden))
{
  revealTiles(9);
}

MapResources::MapResources(SimpleResources resources)
  : Resources(resources)
  , visible(DefaultResources{})
  , hidden(std::move(resources))
{
}

void MapResources::revealTile()
{
  if (numHiddenTiles <= 0)
    return;
  --numHiddenTiles;

  // Except for plants and blood pools, at most own resource is spawned.
  // This ignores some situations, e.g. petrified enemies which are both a wall and a gold pile.
  // The helper functions return the available amount of a hidden resources of a given type and a method to reveal it.
  auto wrap = [&](auto what) {
    return std::pair{hidden.*what, [&from = hidden.*what, &to = visible.*what](size_t) {
                       assert(from > 0);
                       ++to;
                       --from;
                     }};
  };
  auto wrapVector = [&](auto group) {
    return std::pair{(hidden.*group).size(), [&from = hidden.*group, &to = visible.*group](size_t index) {
                       assert(index < from.size());
                       auto resource = begin(from) + index;
                       to.emplace_back(std::move(*resource));
                       from.erase(resource);
                     }};
  };
  std::array<std::pair<int, std::function<void(size_t)>>, 12> resourceCountsAndReveals{
      wrap(&ResourceSet::numWalls),
      wrap(&ResourceSet::numGoldPiles),
      wrapVector(&ResourceSet::shops),
      wrapVector(&ResourceSet::spells),
      wrapVector(&ResourceSet::altars),
      wrap(&ResourceSet::numAttackBoosters),
      wrap(&ResourceSet::numManaBoosters),
      wrap(&ResourceSet::numHealthBoosters),
      wrap(&ResourceSet::numHealthPotions),
      wrap(&ResourceSet::numManaPotions),
      wrap(&ResourceSet::numPotionShops),
      std::pair{hidden.pactMakerAvailable,
                [&](size_t) {
                  visible.pactMakerAvailable = true;
                  hidden.pactMakerAvailable = false;
                }},
  };
  auto rand = std::uniform_int_distribution<>(0, numHiddenTiles);
  auto n = rand(generator);
  bool revealedWall = true;
  for (const auto& [count, reveal] : resourceCountsAndReveals)
  {
    if (n < count)
    {
      reveal(n);
      break;
    }
    n -= count;
    revealedWall = false;
  }
  if (revealedWall)
    return;

  if (hidden.numPlants > 0 && rand(generator) < hidden.numPlants)
  {
    --hidden.numPlants;
    ++visible.numPlants;
  }
  if (hidden.numBloodPools > 0 && rand(generator) < hidden.numBloodPools)
  {
    --hidden.numBloodPools;
    ++visible.numBloodPools;
  }
}
