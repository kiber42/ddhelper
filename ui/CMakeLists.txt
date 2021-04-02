project(ddhelperui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(USE_VULKAN)
  add_library(imguiapp STATIC src/ImguiAppVulkan.cpp)
  target_link_options(imguiapp PUBLIC "-lvulkan")

else()
  add_library(imguiapp STATIC src/ImguiAppOpenGL.cpp)
  if(NOT WIN32)
    target_link_options(imguiapp PUBLIC "-lGL" "-lGLEW")
  else()
    target_sources(imguiapp PRIVATE "${GL3W_INCLUDE_PATH}/GL/gl3w.c")
    target_link_options(imguiapp PUBLIC "-lOpenGL32" "-lglu32"
                        "-Wl,-subsystem:console")
  endif()

endif()

target_include_directories(imguiapp PUBLIC include ../imgui/backends)
target_link_libraries(imguiapp dearimgui)

add_executable(
  ddhelper
  src/Arena.cpp
  src/DDHelper.cpp
  src/HeroSelection.cpp
  src/History.cpp
  src/MonsterPool.cpp
  src/MonsterSelection.cpp
  src/ResourcesUI.cpp
  src/State.cpp
  src/Utils.cpp)
target_link_libraries(ddhelper ddhelperengine ddsolver imguiapp)
target_include_directories(ddhelper PUBLIC imgui)
