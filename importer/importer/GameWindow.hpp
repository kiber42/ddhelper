#pragma once

#if !defined(_WIN32)
struct _XDisplay;
typedef struct _XDisplay Display;
typedef unsigned long XID;
typedef XID Window;
#else
#include <wtypes.h>
typedef HWND Window;
#endif

namespace importer
{
  class GameWindow
  {
  public:
    //! Create GameWindow object, look for Desktop Dungeons window
    GameWindow();
    GameWindow(const GameWindow&) = delete;
    GameWindow& operator()(const GameWindow&) = delete;
    GameWindow(GameWindow&&) = delete;
    GameWindow& operator()(GameWindow&&) = delete;

    //! Return window handle (platform-specific)
    Window getHandle() const { return window; }

    //! Confirm that window handle corresponds to a valid window
    bool valid() const;

#if !defined(_WIN32)
    ~GameWindow();

    //! Return ID of display on which the window was found
    Display* getDisplay() const { return display; }

  private:
    Display* display;
#endif
    Window window;
  };
} // namespace importer
