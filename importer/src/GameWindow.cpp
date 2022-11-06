#include "importer/GameWindow.hpp"

#if !defined(_WIN32)

#include <X11/Xlib.h>

#include <memory>
#include <string>

namespace
{
  Window findWindowByName(Display* display, Window top, const std::string& name)
  {
    char* window_name;
    if (XFetchName(display, top, &window_name) && name == window_name)
      return top;

    Window dummy;
    Window* children;
    unsigned int numChildren;
    if (!XQueryTree(display, top, &dummy, &dummy, &children, &numChildren))
      return 0;

    auto cleaner = [](Window* children) { XFree(children); };
    std::unique_ptr<Window, decltype(cleaner)> cleanup(children);
    for (unsigned i = 0; i < numChildren; ++i)
    {
      if (auto window = findWindowByName(display, children[i], name))
        return window;
    }
    return 0;
  }

  Window findWindowByName(Display* display, const std::string& name)
  {
    for (int i = 0; i < ScreenCount(display); ++i)
    {
      if (auto window = findWindowByName(display, RootWindow(display, i), name))
        return window;
    }
    return 0;
  }
} // namespace

namespace importer
{
  GameWindow::GameWindow()
    : display(XOpenDisplay(NULL))
    , window(findWindowByName(display, "Desktop Dungeons"))
  {
  }

  GameWindow::~GameWindow() { XCloseDisplay(display); }

  bool GameWindow::valid() const
  {
    thread_local XWindowAttributes updated;
    return display && window && XGetWindowAttributes(display, window, &updated);
  }
} // namespace importer

#else

namespace importer
{
  GameWindow::GameWindow()
    : window(FindWindow(NULL, TEXT("Desktop Dungeons")))
  {
  }

  bool GameWindow::valid() const { return IsWindow(window); }
} // namespace importer

#endif