#pragma once

#include <optional>
#include <utility>

namespace importer
{
  class GameWindow;

  using MousePosition = std::pair<int, int>;

  std::optional<MousePosition> getMousePosition(GameWindow&);
  void moveMouseTo(GameWindow&, MousePosition);

  class AutoRestoreMousePosition
  {
  public:
    AutoRestoreMousePosition(GameWindow&);
    ~AutoRestoreMousePosition();

  private:
    GameWindow& gameWindow;
    std::optional<MousePosition> initialMousePosition;
  };
} // namespace importer
