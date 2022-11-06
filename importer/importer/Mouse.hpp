#pragma once

#include "importer/GameWindow.hpp"

#include <optional>
#include <utility>

namespace importer
{
  using MousePosition = std::pair<int, int>;

  std::optional<MousePosition> getMousePosition(GameWindow&);
  void moveMouseTo(GameWindow&, MousePosition);

  class AutoRestoreMousePosition
  {
  public:
    AutoRestoreMousePosition(GameWindow);
    ~AutoRestoreMousePosition();

  private:
    GameWindow gameWindow;
    std::optional<MousePosition> initialMousePosition;
  };
} // namespace importer
