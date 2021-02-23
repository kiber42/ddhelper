#pragma once

enum class Item;
enum class God;
enum class Spell;

#include <vector>

struct Resources
{
  std::vector<Item> shops;
  std::vector<Spell> spells;
  std::vector<God> altars;
  int numBlackTiles = 0;
  bool pactMakerAvailable = false;
};
