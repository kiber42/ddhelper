#pragma once

// TODO: Implement mouse positioning on Windows
#if !defined(_WIN32)
#include <optional>
#include <utility>

struct _XDisplay;
typedef struct _XDisplay Display;

typedef unsigned long XID;
typedef XID Window;

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
