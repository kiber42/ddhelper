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
#if !defined(_WIN64)
    ActionResultUI operator()();

  private:
    Dungeon selectedDungeon{Dungeon::HobblersHold};
    unsigned acquireHitPointsMode{2};
    std::string status;

#else
    // Stub for Windows, the importer package is currently Linux only
    ActionResultUI operator()() { return std::nullopt; }
#endif
  };
} // namespace ui
