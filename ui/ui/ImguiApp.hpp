#pragma once

#include <memory>
#include <string>

#include "imgui.h"
#include <SDL.h>

namespace ui
{
  class ImguiApp
  {
  public:
    ImguiApp(const std::string& windowTitle);
    ImguiApp(const ImguiApp&) = delete;
    ImguiApp& operator=(const ImguiApp&) = delete;
    ~ImguiApp();

    void run();

  protected:
    ImVec4 background_color;

    virtual void processEvent(SDL_Event&) {}
    virtual void populateFrame() {}

  private:
    struct Data;
    std::unique_ptr<Data> data;
  };
} // namespace ui
