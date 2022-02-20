#pragma once

#include <X11/Xlib.h>

#include <memory>
#include <optional>

class GameWindow
{
public:
  GameWindow();

  Display* getDisplay() const { return display.get(); }

  Window getWindow() const { return window.value_or(0); }

  bool valid() const;

private:
  struct display_cleanup
  {
    void operator()(Display* display) { XCloseDisplay(display); }
  };
  std::unique_ptr<Display, display_cleanup> display;
  std::optional<Window> window;
};
