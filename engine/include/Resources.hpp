#pragma once

#include "Faith.hpp"
#include "Items.hpp"
#include "Spells.hpp"

#include <vector>

struct Resources
{
  std::vector<Item> shops;
  std::vector<Spell> spells;
  std::vector<God> altars;
  int numBlackTiles = 0;
  bool pactMakerAvailable = false;
};
