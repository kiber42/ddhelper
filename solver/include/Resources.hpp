#pragma once

#include "Faith.hpp"
#include "Items.hpp"
#include "Spells.hpp"

#include <vector>

struct Resources
{
  int numBlackTiles;
  std::vector<Item> shops;
  std::vector<Spell> spells;
  std::vector<God> altars;
  bool pactMakerAvailable;
};
