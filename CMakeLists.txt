cmake_minimum_required(VERSION 3.10)

# set the project name
project(ddhelper VERSION 0.3.5)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# compile with warnings
message(CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion -Wno-implicit-int-float-conversion -Wno-sign-conversion")
else()
  # Numerous conversion warnings from GCC in imgui; no flag to only disable int-to-float conversion warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-conversion -Wno-return-type")
endif()

set(USE_VULKAN OFF)

if(NOT WIN32)
  find_package(SDL2 REQUIRED)
else()
  # Windows: Assume that SDL has been unpacked directly into project directory
  set(SDL2_INCLUDE_DIRS "SDL/include")
  set(SDL2_LIBRARIES "SDL/lib/x64")
  # Use gl3w libary shipped with imgui
  get_filename_component(GL3W_INCLUDE_PATH imgui/examples/libs/gl3w ABSOLUTE .)
  # No Vulkan build for Windows
  set(USE_VULKAN OFF)
endif()

add_library(
  dearimgui STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_sdl.cpp)
target_include_directories(dearimgui PUBLIC imgui "${SDL2_INCLUDE_DIRS}" "${GL3W_INCLUDE_PATH}")
target_link_libraries(dearimgui ${OPENGL_gl_LIBRARY} "${SDL2_LIBRARIES}")
if(USE_VULKAN)
  target_sources(dearimgui PRIVATE imgui/backends/imgui_impl_vulkan.cpp)
else()
  target_link_libraries(dearimgui ${CMAKE_DL_LIBS})
  target_sources(dearimgui PRIVATE imgui/backends/imgui_impl_opengl3.cpp)
endif()

add_executable(imguidemo imgui/imgui_demo.cpp)
target_link_libraries(imguidemo imguiapp)
if(USE_VULKAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-conversion")
  target_sources(imguidemo PRIVATE imgui/examples/example_sdl_vulkan/main.cpp)
else()
  target_sources(imguidemo PRIVATE imgui/examples/example_sdl_opengl3/main.cpp)
endif()

add_subdirectory(engine)
add_subdirectory(ui)
add_subdirectory(solver)
if (NOT WIN32)
  add_subdirectory(importer)
endif()
