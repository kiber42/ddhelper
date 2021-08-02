#include "bridge/Mouse.hpp"

#include <cstdio>

#include "X11/X.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"

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
  printf("Move mouse to %i, %i\n", x, y);
  if (window == 0)
    window = DefaultRootWindow(display);
  XWarpPointer(display, None, window, 0, 0, 0, 0, x, y);
  XFlush(display);
}
