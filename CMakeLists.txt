cmake_minimum_required(VERSION 3.10)

# set the project name
project(ddhelper VERSION 0.2.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(USE_VULKAN OFF)

add_library(
    dearimgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/examples/imgui_impl_sdl.cpp
)
target_include_directories(dearimgui PUBLIC imgui /usr/include/SDL2)
target_link_options(dearimgui PUBLIC "-lSDL2")
if(USE_VULKAN)
target_sources(dearimgui PRIVATE imgui/examples/imgui_impl_vulkan.cpp)
else()
target_sources(dearimgui PRIVATE imgui/examples/imgui_impl_opengl3.cpp)
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
