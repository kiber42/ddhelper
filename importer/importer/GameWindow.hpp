#pragma once

#include <memory>
#include <optional>

struct _XDisplay;
typedef struct _XDisplay Display;

typedef unsigned long XID;
typedef XID Window;

namespace importer
{
  class GameWindow
  {
  public:
    GameWindow();
    ~GameWindow();

    Display* getDisplay() const { return display; }

    Window getWindow() const { return window.value_or(0); }

    bool valid() const;

  private:
    Display* display;
    std::optional<Window> window;
  };
} // namespace importer
