#include "importer/GameWindow.hpp"

#include <string>

namespace
{
  std::optional<Window> findWindowByName(Display* display, Window top, const std::string& name)
  {
    char* window_name;
    if (XFetchName(display, top, &window_name) && name == window_name)
      return top;

    Window dummy;
    Window* children;
    unsigned int numChildren;
    if (!XQueryTree(display, top, &dummy, &dummy, &children, &numChildren))
      return {};

    auto cleaner = [](Window* children) { XFree(children); };
    std::unique_ptr<Window, decltype(cleaner)> cleanup(children);
    for (unsigned i = 0; i < numChildren; ++i)
    {
      if (auto window = findWindowByName(display, children[i], name))
        return window;
    }
    return {};
  }

  std::optional<Window> findWindowByName(Display* display, const std::string& name)
  {
    for (int i = 0; i < ScreenCount(display); ++i)
    {
      if (auto window = findWindowByName(display, RootWindow(display, i), name))
        return window;
    }
    return {};
  }
} // namespace

GameWindow::GameWindow()
  : display(XOpenDisplay(NULL))
  , window(findWindowByName(display.get(), "Desktop Dungeons"))
{
}

bool GameWindow::valid() const
{
  thread_local XWindowAttributes updated;
  return display && window && XGetWindowAttributes(display.get(), *window, &updated);
}
