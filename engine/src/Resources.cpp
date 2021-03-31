#include "Resources.hpp"

void MapResources::revealTile()
{
  if (numHiddenTiles <= 0)
    return;
  --numHiddenTiles;

  auto rand = std::uniform_int_distribution<>(0, numHiddenTiles);
  if (hidden.numWalls > 0 && rand(generator) < hidden.numWalls)
  {
    --hidden.numWalls;
    ++visible.numWalls;
    // Simplification: A tile with a wall cannot contain any other resource (this ignores petrified enemies)
    return;
  }

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
  if (hidden.numGoldPiles > 0 && rand(generator) < hidden.numGoldPiles)
  {
    --hidden.numGoldPiles;
    ++visible.numGoldPiles;
  }

  // Can have at most one shop, spell, or altar per tile
  const int numShops = hidden.shops.size();
  const int numSpells = hidden.spells.size();
  const int numGods = hidden.altars.size();
  const int n = numShops + numSpells + numGods + (int)(hidden.pactMakerAvailable);
  if (n > 0)
  {
    const int r = rand(generator);
    if (r < n)
    {
      if (r < numShops)
      {
        auto shop = hidden.shops.begin() + r;
        visible.shops.emplace_back(std::move(*shop));
        hidden.shops.erase(shop);
      }
      else if (r - numShops < numSpells)
      {
        auto spell = hidden.spells.begin() + (r - numShops);
        visible.spells.emplace_back(std::move(*spell));
        hidden.spells.erase(spell);
      }
      else if (r - numShops - numSpells < numGods)
      {
        auto altar = hidden.altars.begin() + (r - numShops - numSpells);
        visible.altars.emplace_back(std::move(*altar));
        hidden.altars.erase(altar);
      }
      else
      {
        hidden.pactMakerAvailable = false;
        visible.pactMakerAvailable = true;
      }
    }
  }
}
