cmake_minimum_required(VERSION 3.10)

# set the project name
project(ddhelper VERSION 0.3.5)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# compile with warnings
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion -Wno-implicit-int-float-conversion")
endif()

set(USE_VULKAN OFF)

if(NOT WIN32)
  set(SDL_INCLUDE_PATH /usr/include/SDL2)
else()
  # Windows: Assume that SDL has been unpacked directly into project directory
  set(SDL_INCLUDE_PATH SDL/include)
  link_directories(SDL/lib/x64)
  # Use gl3w libary shipped with imgui
  get_filename_component(GL3W_INCLUDE_PATH imgui/examples/libs/gl3w ABSOLUTE .)
  # No Vulkan build for Windows
  set(USE_VULKAN OFF)
endif()

add_library(
  dearimgui STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_sdl.cpp)
target_include_directories(dearimgui PUBLIC imgui ${SDL_INCLUDE_PATH}
                                            ${GL3W_INCLUDE_PATH})
target_link_libraries(dearimgui "-lSDL2" "-lSDL2main")
if(USE_VULKAN)
  target_sources(dearimgui PRIVATE imgui/backends/imgui_impl_vulkan.cpp)
else()
  target_sources(dearimgui PRIVATE imgui/backends/imgui_impl_opengl3.cpp)
endif()

add_executable(imguidemo imgui/imgui_demo.cpp)
target_link_libraries(imguidemo imguiapp)
if(USE_VULKAN)
  target_sources(imguidemo PRIVATE imgui/examples/example_sdl_vulkan/main.cpp)
else()
  target_sources(imguidemo PRIVATE imgui/examples/example_sdl_opengl3/main.cpp)
endif()

add_subdirectory(engine)
add_subdirectory(ui)
add_subdirectory(solver)
add_subdirectory(importer)
