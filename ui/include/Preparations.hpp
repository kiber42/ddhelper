#pragma once

#include "Hero.hpp"
#include "Resources.hpp"
#include "Utils.hpp"

#include <optional>

namespace ui
{
  class Preparations
  {
  public:
    Preparations(int mapSize = DefaultMapSize);

    ActionResultUI run(const State& state);

  private:
    ActionResultUI result;

    int selectedPopupItem;
  };
} // namespace ui
