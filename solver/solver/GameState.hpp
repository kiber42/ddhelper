#pragma once

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Resources.hpp"

#include <vector>

struct GameState
{
  Hero hero{};
  Monsters monsters{};
  SimpleResources resources{20};
};
