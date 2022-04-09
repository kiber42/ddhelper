#pragma once

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Resources.hpp"

struct GameState
{
  Hero hero{};
  Monsters visibleMonsters{};
  HiddenMonsters hiddenMonsters{};
  std::size_t activeMonster{0};
  SimpleResources resources{ResourceSet{DungeonSetup{}}, 20};
};
