#pragma once

#include <string>
#include <memory>

#include "imgui.h"
#include <SDL.h>

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

  virtual void processEvent(SDL_Event&) {};
  virtual void populateFrame() {};

private:
  class Data;
  std::unique_ptr<Data> data;
};
