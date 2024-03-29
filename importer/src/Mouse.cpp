#include "importer/Mouse.hpp"

#include "importer/GameWindow.hpp"

#if !defined(_WIN32)
#include "X11/X.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#else
#include <WinUser.h>
#endif

namespace importer
{
#if !defined(_WIN32)
  std::optional<MousePosition> getMousePosition(GameWindow& gameWindow)
  {
    auto display = gameWindow.getDisplay();
    auto window = gameWindow.getHandle();
    if (window == 0)
      window = DefaultRootWindow(display);
    Window root, child;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask;
    if (XQueryPointer(display, window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask))
      return std::pair{root_x, root_y};
    return {};
  }

  void moveMouseTo(GameWindow& gameWindow, MousePosition position)
  {
    auto display = gameWindow.getDisplay();
    auto window = gameWindow.getHandle();
    if (window == 0)
      window = DefaultRootWindow(display);
    XWarpPointer(display, None, window, 0, 0, 0, 0, position.first, position.second);
    XFlush(display);
  }
#else
  std::optional<MousePosition> getMousePosition(GameWindow& gameWindow)
  {
    POINT p;
    if (!gameWindow.valid() || !GetCursorPos(&p))
      return {};
    ScreenToClient(gameWindow.getHandle(), &p);
    return std::pair{static_cast<int>(p.x), static_cast<int>(p.y)};
  }

  void moveMouseTo(GameWindow& gameWindow, MousePosition pos)
  {
    if (!gameWindow.valid())
      return;
    POINT p {.x = pos.first, .y = pos.second };
    ClientToScreen(gameWindow.getHandle(), &p);
    SetCursorPos(p.x, p.y);
  }
#endif

  AutoRestoreMousePosition::AutoRestoreMousePosition(GameWindow& gameWindow)
    : gameWindow(gameWindow)
    , initialMousePosition(getMousePosition(gameWindow))
  {
  }

  AutoRestoreMousePosition::~AutoRestoreMousePosition()
  {
    if (initialMousePosition)
      moveMouseTo(gameWindow, *initialMousePosition);
  }
} // namespace importer
