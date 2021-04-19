#pragma once

#include "Hero.hpp"
#include "Resources.hpp"
#include "Utils.hpp"

#include <optional>

namespace ui
{
  class Resources
  {
  public:
    Resources(int mapSize = DefaultMapSize);

    ActionResultUI run(const State& state);

  private:
    ActionResultUI result;

    int selectedPopupItem;
    int mapSize;

    void runSpawnShop(const State& state);
    void runSpawnSpell(const State& state);
    void runSpawnAltar(const State& state);
    void runCheat(const State& state);
  };
} // namespace ui
