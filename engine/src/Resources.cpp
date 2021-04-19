#include "engine/Resources.hpp"

#include "engine/Faith.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

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
  addRandomResources(8, 5, 3);
}

ResourceSet::ResourceSet(const std::set<ResourceModifier>& modifiers, std::optional<God> preparedDeity, int mapSize)
  : numWalls{mapSize * mapSize * 4 / 10 * (preparedDeity == God::BinlorIronshield ? 7 : 10) / 10}
  , numHealthPotions{3}
  , numManaPotions{3}
  , numPotionShops{modifiers.count(ResourceModifier::Apothecary) ? 3 : 1}
  , numAttackBoosters{modifiers.count(ResourceModifier::ExtraAttackBoosters) ? 5 : 3}
  , numManaBoosters{modifiers.count(ResourceModifier::ExtraManaBoosters) ? 5 : 3}
  , numHealthBoosters{modifiers.count(ResourceModifier::ExtraHealthBoosters) ? 5 : 3}
  , numGoldPiles{10}
{
  int numShops = modifiers.count(ResourceModifier::Merchant) ? 10 : 8;
  int numSpells =
      modifiers.count(ResourceModifier::ExtraGlyph) ? 6 : modifiers.count(ResourceModifier::FewerGlyphs) ? 4 : 5;
  const int numAltars = 3 + modifiers.count(ResourceModifier::Martyr) + modifiers.count(ResourceModifier::ExtraAltar);
  if (modifiers.count(ResourceModifier::Hoarder))
  {
    numHealthPotions += numHealthPotions / 3;
    numManaPotions += numManaPotions / 3;
    numAttackBoosters += numAttackBoosters / 3;
    numManaBoosters += numManaBoosters / 3;
    numHealthBoosters += numHealthBoosters / 3;
    numGoldPiles += numGoldPiles / 3;
    numShops += numShops / 3;
    numSpells += numSpells / 3;
  }
  addRandomResources(numShops, numSpells, numAltars);
  if (preparedDeity && std::find(begin(altars), end(altars), GodOrPactmaker{*preparedDeity}) == end(altars))
  {
    if (!altars.empty())
      altars.pop_back();
    altars.push_back(*preparedDeity);
  }
  if (modifiers.count(ResourceModifier::FlameMagnet))
    spells.erase(std::find(begin(spells), end(spells), Spell::Burndayraz));
}

bool ResourceSet::pactmakerAvailable() const
{
  return std::find(begin(altars), end(altars), GodOrPactmaker{Pactmaker::ThePactmaker}) != end(altars);
}

void ResourceSet::addRandomShop(std::mt19937 generator)
{
  const int n = static_cast<int>(Item::LastShopItem);
  if (shops.size() > n)
    return;
  Item item;
  do
  {
    item = static_cast<Item>(std::uniform_int_distribution<>(0, n)(generator));
  } while (std::find(begin(shops), end(shops), item) != end(shops));
  shops.emplace_back(item);
}

void ResourceSet::addRandomSpell(std::mt19937 generator)
{
  const int n = static_cast<int>(Spell::Last);
  if (spells.size() > n)
    return;
  Spell spell;
  do
  {
    spell = static_cast<Spell>(std::uniform_int_distribution<>(0, n)(generator));
  } while (std::find(begin(spells), end(spells), spell) != end(spells));
  spells.emplace_back(spell);
}

void ResourceSet::addRandomAltar(std::mt19937 generator)
{
  const unsigned n = static_cast<int>(God::Last) + 1;
  if (altars.size() > n)
    return;
  GodOrPactmaker god;
  do
  {
    const unsigned value = std::uniform_int_distribution<>(0, n)(generator);
    if (value < n)
      god = static_cast<God>(value);
    else
      god = Pactmaker::ThePactmaker;
  } while (std::find(begin(altars), end(altars), god) != end(altars));
  altars.emplace_back(god);
}

void ResourceSet::addRandomResources(int numShops, int numSpells, int numAltars)
{
  std::mt19937 generator{std::random_device{}()};
  for (int i = 0; i < numShops; ++i)
    addRandomShop(generator);
  for (int i = 0; i < numSpells; ++i)
    addRandomSpell(generator);
  for (int i = 0; i < numAltars; ++i)
    addRandomAltar(generator);
  // Burndayraz is guaranteed to appear
  if (std::find(begin(spells), end(spells), Spell::Burndayraz) == end(spells))
  {
    if (!spells.empty())
      spells.pop_back();
    spells.push_back(Spell::Burndayraz);
    std::shuffle(begin(spells), end(spells), generator);
  }
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
  ++numRevealedTiles;

  // Except for plants and blood pools, at most own resource is spawned.
  // This ignores some situations, e.g. petrified enemies which are both a wall and a gold pile.
  // The helper functions return the available amount of a hidden resources of a given type and a method to reveal it.
  auto countAndReveal = [&](auto what) {
    return std::pair{hidden.*what, [&from = hidden.*what, &to = visible.*what](size_t) {
                       assert(from > 0);
                       ++to;
                       --from;
                     }};
  };
  auto countAndRevealVector = [&](auto group) {
    return std::pair{(hidden.*group).size(), [&from = hidden.*group, &to = visible.*group](size_t index) {
                       assert(index < from.size());
                       auto resource = begin(from) + index;
                       to.emplace_back(std::move(*resource));
                       from.erase(resource);
                     }};
  };
  std::array<std::pair<int, std::function<void(size_t)>>, 12> resourceCountsAndReveals{
      countAndReveal(&ResourceSet::numWalls),         countAndReveal(&ResourceSet::numGoldPiles),
      countAndRevealVector(&ResourceSet::shops),      countAndRevealVector(&ResourceSet::spells),
      countAndRevealVector(&ResourceSet::altars),     countAndReveal(&ResourceSet::numAttackBoosters),
      countAndReveal(&ResourceSet::numManaBoosters),  countAndReveal(&ResourceSet::numHealthBoosters),
      countAndReveal(&ResourceSet::numHealthPotions), countAndReveal(&ResourceSet::numManaPotions),
      countAndReveal(&ResourceSet::numPotionShops)};
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
