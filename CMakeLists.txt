cmake_minimum_required(VERSION 3.10)

# set the project name
project(DDHelperTest VERSION 0.1.0)

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


if(USE_VULKAN)
add_library(imguiapp STATIC src/ImguiAppVulkan.cpp)
target_link_options(imguiapp PUBLIC "-lvulkan")
else()
add_library(imguiapp STATIC src/ImguiAppOpenGL.cpp)
target_link_libraries(imguiapp dearimgui)
target_link_options(imguiapp PUBLIC "-lGL" "-lGLEW")
endif()
target_include_directories(imguiapp PUBLIC include)
target_link_libraries(imguiapp dearimgui)


add_library(
    DDHelperLibrary STATIC
    src/Combat.cpp
    src/Defence.cpp
    src/Dungeon.cpp
    src/Experience.cpp
    src/Faith.cpp    
    src/Hero.cpp
    src/HeroStats.cpp
    src/ItemClass.cpp
    src/Jehora.cpp
    src/Monster.cpp
    src/MonsterStats.cpp
    src/MonsterStatus.cpp
    src/MonsterTraits.cpp
    src/MonsterTypes.cpp
    src/Spells.cpp
)
target_include_directories(DDHelperLibrary PUBLIC include)


add_executable(DDHelperTest src/DDHelperTest.cpp)
target_include_directories(DDHelperTest PRIVATE bandit)
target_link_libraries(DDHelperTest DDHelperLibrary)


add_executable(DDHelper src/DDHelper.cpp)
target_link_libraries(DDHelper DDHelperLibrary)
target_link_libraries(DDHelper imguiapp)
target_include_directories(DDHelper PUBLIC imgui)
target_include_directories(DDHelper PUBLIC /usr/include/SDL2)


add_executable(imguidemo imgui/imgui_demo.cpp)
target_link_libraries(imguidemo imguiapp)
if(USE_VULKAN)
target_sources(imguidemo PRIVATE imgui/examples/example_sdl_vulkan/main.cpp)
else()
target_sources(imguidemo PRIVATE imgui/examples/example_sdl_opengl3/main.cpp)
endif()
