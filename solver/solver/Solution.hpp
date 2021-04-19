#pragma once

#include "engine/Faith.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

#include <string>
#include <variant>
#include <vector>

struct Attack
{
};

struct Cast
{
  Spell spell;
};

struct Uncover
{
  int numTiles;
};

struct Buy
{
  Item item;
};

struct Use
{
  Item item;
};

struct Convert
{
  std::variant<Item, Spell> itemOrSpell;
};

struct Find
{
  Spell spell;
};

struct Follow
{
  God deity;
};

struct Request
{
  std::variant<Boon, Pact> boonOrPact;
};

struct Desecrate
{
  God altar;
};

using Step = std::variant<Attack, Cast, Uncover, Buy, Use, Convert, Find, Follow, Request, Desecrate>;

using Solution = std::vector<Step>;

std::string toString(const Step&);
std::string toString(const Solution&);

bool isCombat(const Step&);
