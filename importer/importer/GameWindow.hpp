#pragma once

#include <memory>
#include <optional>

#if !defined(_WIN32)
struct _XDisplay;
typedef struct _XDisplay Display;

typedef unsigned long XID;
typedef XID Window;
#endif

namespace importer
{
  class GameWindow
  {
#if !defined(_WIN32)
  public:
    GameWindow();
    ~GameWindow();

    Display* getDisplay() const { return display; }

    Window getWindow() const { return window.value_or(0); }

    bool valid() const;

  private:
    Display* display;
    std::optional<Window> window;
#else
  public:
    // TODO: Stub for Windows
    bool valid() const { return true; }
#endif
  };
} // namespace importer
