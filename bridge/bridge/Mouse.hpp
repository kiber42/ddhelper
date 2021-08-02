#pragma once

#include "X11/Xlib.h"

#include <optional>
#include <utility>

std::optional<std::pair<int, int>> getMousePosition(Display* display, Window window);
void moveMouseTo(Display* display, Window window, int x, int y);
