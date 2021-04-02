#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Resources.hpp"

#include <vector>

struct GameState
{
  Hero hero{};
  Monsters monsters{};
  SimpleResources resources{};
};
