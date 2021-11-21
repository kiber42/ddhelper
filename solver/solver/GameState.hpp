#pragma once

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Resources.hpp"

#include <vector>

struct GameState
{
  Hero hero{};
  Monsters visibleMonsters{};
  HiddenMonsters hiddenMonsters{};
  SimpleResources resources{ResourceSet{DungeonSetup{}}, 20};
};
