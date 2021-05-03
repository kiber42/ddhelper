#pragma once

#include "ui/Utils.hpp"

#include "engine/Hero.hpp"
#include "engine/Resources.hpp"

#include <optional>

namespace ui
{
  class Resources
  {
  public:
    ActionResultUI run(const State& state);

  private:
    ActionResultUI result;

    int selectedPopupItem;

    void runSpawnShop(const State& state);
    void runSpawnSpell(const State& state);
    void runSpawnAltar(const State& state);
    void runCheat(const State& state);
  };
} // namespace ui
