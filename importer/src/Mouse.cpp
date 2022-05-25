#include "importer/Mouse.hpp"

#if !defined(_WIN32)
#include "importer/GameWindow.hpp"

#include <cstdio>

#include "X11/X.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"

namespace importer
{
  std::optional<std::pair<int, int>> getMousePosition(Display* display, Window window)
  {
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

  void moveMouseTo(Display* display, Window window, int x, int y)
  {
    if (window == 0)
      window = DefaultRootWindow(display);
    XWarpPointer(display, None, window, 0, 0, 0, 0, x, y);
    XFlush(display);
  }

  AutoRestoreMousePosition::AutoRestoreMousePosition(Display* display)
    : display(display)
    , initialMousePosition(getMousePosition(display, 0))
  {
  }

  AutoRestoreMousePosition::AutoRestoreMousePosition(GameWindow& gameWindow)
    : AutoRestoreMousePosition(gameWindow.getDisplay())
  {
  }

  AutoRestoreMousePosition::~AutoRestoreMousePosition()
  {
    if (initialMousePosition)
      moveMouseTo(display, 0, initialMousePosition->first, initialMousePosition->second);
  }
} // namespace importer
#endif
