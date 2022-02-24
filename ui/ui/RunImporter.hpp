#pragma once

#include "ui/State.hpp"
#include "ui/Utils.hpp"

#include "importer/GameWindow.hpp"
#include "importer/ImageCapture.hpp"
#include "importer/ImageProcessor.hpp"

#include <string>

namespace ui
{
  class RunImporter
  {
  public:
    ActionResultUI operator()();

  private:
    unsigned selectedDungeonIndex{1};
    unsigned acquireHitPointsMode{2};
    std::string status;
  };
} // namespace ui
