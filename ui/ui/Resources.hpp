#pragma once

#include "ui/State.hpp"
#include "ui/Utils.hpp"

namespace ui
{
  class Resources
  {
  public:
    ActionResultUI run(const State& state);

  private:
    ActionResultUI result;

    int selectedPopupItem{-1};

    void runSpawnShop(const State& state);
    void runSpawnSpell(const State& state);
    void runSpawnAltar(const State& state);
    void runCheat(const State& state);
  };
} // namespace ui
