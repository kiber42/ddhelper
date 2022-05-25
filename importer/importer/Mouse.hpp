#pragma once

// TODO: Implement mouse positioning on Windows
#if !defined(_WIN32)
#include "X11/Xlib.h"

#include <optional>
#include <utility>

namespace importer
{
  using MousePosition = std::pair<int, int>;

  std::optional<MousePosition> getMousePosition(Display*, Window);

  void moveMouseTo(Display* display, Window window, int x, int y);

  class GameWindow;

  class AutoRestoreMousePosition
  {
  public:
    AutoRestoreMousePosition(Display*);
    AutoRestoreMousePosition(GameWindow&);
    ~AutoRestoreMousePosition();

  private:
    Display* display;
    std::optional<MousePosition> initialMousePosition;
  };
} // namespace importer
#endif
