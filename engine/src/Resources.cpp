#include "engine/Resources.hpp"

#include "engine/Faith.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

#include <algorithm>
#include <cassert>

ResourceSet::ResourceSet(DungeonSetup setup)
  : numWalls{setup.mapSize * setup.mapSize * 4 * (setup.altar == GodOrPactmaker{God::BinlorIronshield} ? 7u : 10u) / 100}
  , numHealthPotions{3}
  , numManaPotions{3}
  , numPotionShops{setup.modifiers.count(BazaarModifier::Apothecary) ? 3u : 1u}
  , numAttackBoosters{setup.modifiers.count(MageModifier::ExtraAttackBoosters) ? 5u : 3u}
  , numManaBoosters{setup.modifiers.count(MageModifier::ExtraManaBoosters) ? 5u : 3u}
  , numHealthBoosters{setup.modifiers.count(MageModifier::ExtraHealthBoosters) ? 5u : 3u}
  , numGoldPiles{10}
{
  int numShops = hasStartingTrait(setup.heroClass, HeroTrait::Merchant) ? 10 : 8;
  int numSpells =
      setup.modifiers.count(MageModifier::ExtraGlyph) ? 6 : (setup.modifiers.count(MageModifier::FewerGlyphs) ? 4 : 5);
  const int numAltars = 3 + (int)hasStartingTrait(setup.heroClass, HeroTrait::Martyr) +
                        (int)(setup.altar == GodOrPactmaker{Pactmaker::ThePactmaker});
  if (hasStartingTrait(setup.heroClass, HeroTrait::Hoarder))
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
  if (setup.altar && std::find(begin(altars), end(altars), *setup.altar) == end(altars))
  {
    if (!altars.empty())
      altars.pop_back();
    altars.push_back(*setup.altar);
  }
  // Flame magnet moves Burndayraz from the map to the player's inventory
  if (setup.modifiers.count(MageModifier::FlameMagnet))
    spells.erase(std::find(begin(spells), end(spells), Spell::Burndayraz));
}

bool ResourceSet::pactmakerAvailable() const
{
  return std::find(begin(altars), end(altars), GodOrPactmaker{Pactmaker::ThePactmaker}) != end(altars);
}

void ResourceSet::addRandomShop(std::mt19937 generator)
{
  const int n = static_cast<int>(ShopItem::Last);
  if (shops.size() > n)
    return;
  Item item;
  do
  {
    item = Item{static_cast<ShopItem>(std::uniform_int_distribution<>(0, n)(generator))};
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
    const unsigned value = std::uniform_int_distribution<unsigned>(0, n)(generator);
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

Resources::Resources(unsigned char mapSize)
  : numHiddenTiles(mapSize * mapSize)
{
}

void Resources::revealTiles(unsigned n)
{
  for (; n > 0 && numHiddenTiles > 0; --n)
    revealTile();
}

SimpleResources::SimpleResources(ResourceSet visible, unsigned char mapSize)
  : Resources(mapSize)
  , ResourceSet(std::move(visible))
  , mapSize(mapSize)
{
}

MapResources::MapResources(ResourceSet visible, ResourceSet hidden, unsigned char mapSize)
  : Resources(mapSize)
  , visible(std::move(visible))
  , hidden(std::move(hidden))
{
  revealTiles(9);
}

MapResources::MapResources(unsigned char mapSize)
  : MapResources(DungeonSetup{HeroClass::Guard, HeroRace::Human, mapSize})
{
}

MapResources::MapResources(DungeonSetup dungeonSetup)
  : MapResources(ResourceSet{}, ResourceSet{dungeonSetup}, dungeonSetup.mapSize)
{
}

MapResources::MapResources(SimpleResources resources)
  : Resources(resources)
  , visible()
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
  auto countAndReveal = [&](auto resourceAmount) {
    return std::pair{hidden.*resourceAmount, [&from = hidden.*resourceAmount, &to = visible.*resourceAmount](unsigned) {
                       assert(from > 0);
                       ++to;
                       --from;
                     }};
  };
  auto countAndRevealVector = [&](auto group) {
    return std::pair{(hidden.*group).size(), [&from = hidden.*group, &to = visible.*group](unsigned index) {
                       assert(index < from.size());
                       auto resource = begin(from) + index;
                       to.emplace_back(std::move(*resource));
                       from.erase(resource);
                     }};
  };
  std::array<std::pair<unsigned, std::function<void(unsigned)>>, 12> resourceCountsAndReveals{
      countAndReveal(&ResourceSet::numWalls),         countAndReveal(&ResourceSet::numGoldPiles),
      countAndRevealVector(&ResourceSet::shops),      countAndRevealVector(&ResourceSet::spells),
      countAndRevealVector(&ResourceSet::altars),     countAndReveal(&ResourceSet::numAttackBoosters),
      countAndReveal(&ResourceSet::numManaBoosters),  countAndReveal(&ResourceSet::numHealthBoosters),
      countAndReveal(&ResourceSet::numHealthPotions), countAndReveal(&ResourceSet::numManaPotions),
      countAndReveal(&ResourceSet::numPotionShops)};
  auto rand = std::uniform_int_distribution<unsigned>(0, numHiddenTiles);
  // Use n as an index into the 'cumulative resource function':
  // for n            < numWalls                -> reveal a wall
  // for n - numWalls < numGoldPiles            -> reveal a gold pile
  // for n - numWalls - numGoldPiles < numShops -> reveal a shop
  // ...
  // numOfAllResources <= n < numHiddenTiles -> reveal nothing
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

  // if the revealed tile isn't a wall, it may also contain a plant and/or a blood pool
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
