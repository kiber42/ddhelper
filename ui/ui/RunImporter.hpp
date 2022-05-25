#pragma once

#include "engine/DungeonInfo.hpp"

#include "ui/State.hpp"
#include "ui/Utils.hpp"

#include <string>

namespace ui
{
  class RunImporter
  {
  public:
    ActionResultUI operator()();

  private:
    Dungeon selectedDungeon{Dungeon::HobblersHold};
    unsigned acquireHitPointsMode{2};
    std::string status;
  };
} // namespace ui
