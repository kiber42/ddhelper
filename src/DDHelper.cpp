#include "ImguiApp.hpp"

extern int imgui_sdl_init();
extern void imgui_sdl_main();
extern void imgui_sdl_cleanup();

int main()
{
  ImguiApp app("Dear ImGui SDL2+Vulkan example");
  app.run();
}
