project(ddhelperui VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(imguiapp STATIC src/ImguiAppOpenGL.cpp)
target_include_directories(imguiapp PUBLIC . ../imgui/backends)
target_link_libraries(imguiapp dearimgui)
if(NOT WIN32)
  target_link_libraries(imguiapp "-lGL" "-lGLEW")
else()
  target_link_libraries(imguiapp "-lOpenGL32" "-lglu32")
  target_link_options(imguiapp PUBLIC "-Wl,-subsystem:console")
endif()

add_executable(
  ddhelper
  src/Arena.cpp
  src/DDHelper.cpp
  src/HeroSelection.cpp
  src/History.cpp
  src/MonsterPool.cpp
  src/MonsterSelection.cpp
  src/Resources.cpp
  src/RunImporter.cpp
  src/RunSolver.cpp
  src/State.cpp
  src/Utils.cpp)
target_link_libraries(ddhelper ddhelperengine ddsolver ddimporter imguiapp)
target_include_directories(ddhelper PUBLIC imgui)

# The imgui demo program for SDL+OpenGL3 does not link properly on Windows.
# It is not a mandatory part of this package.
if(NOT WIN32)
  add_executable(imguidemo ../imgui/imgui_demo.cpp ../imgui/examples/example_sdl_opengl3/main.cpp)
  target_link_libraries(imguidemo imguiapp)
endif()
