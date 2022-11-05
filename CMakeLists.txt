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
  # Do not warn about "not all code paths return a value" (all enum values are covered in the switch statements)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /W3 /wd4715")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion -Wno-implicit-int-float-conversion -Wno-sign-conversion")
else()
  # Numerous conversion warnings from GCC in imgui; no flag to only disable int-to-float conversion warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-conversion -Wno-return-type")
endif()

if(NOT WIN32)
  find_package(SDL2 REQUIRED)
else()
  # Windows: Assume that SDL has been unpacked directly into project directory
  get_filename_component(SDL2_PATH "SDL" ABSOLUTE .)
  set(SDL2_INCLUDE_DIRS "${SDL2_PATH}/include")
  set(SDL2_LIBRARIES "${SDL2_PATH}/lib/x64/SDL2.lib")
endif()

add_library(
  dearimgui STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_sdl.cpp
  imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(dearimgui PUBLIC imgui imgui/backends "${SDL2_INCLUDE_DIRS}")
target_link_libraries(dearimgui PUBLIC ${OPENGL_gl_LIBRARY} "${SDL2_LIBRARIES}" ${CMAKE_DL_LIBS})
if (NOT WIN32)
target_link_libraries(dearimgui PUBLIC "-lGL" "-lGLEW")
endif()

add_executable(imguidemo imgui/examples/example_sdl_opengl3/main.cpp imgui/imgui_demo.cpp)
target_compile_options(imguidemo PUBLIC "-DSDL_MAIN_HANDLED")
target_link_libraries(imguidemo dearimgui)

add_subdirectory(engine)
add_subdirectory(ui)
add_subdirectory(solver)
add_subdirectory(importer)
