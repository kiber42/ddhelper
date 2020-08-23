cmake_minimum_required(VERSION 3.10)

# set the project name
project(DDHelperTest VERSION 0.1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(
    DDHelperLibrary STATIC
    src/Attack.cpp
    src/BaseStats.cpp
    src/Defence.cpp
    src/Dungeon.cpp
    src/Experience.cpp
    src/Hero.cpp
    src/HeroStats.cpp
    src/ItemClass.cpp
    src/Melee.cpp
    src/Monster.cpp
    src/MonsterFactory.cpp
    src/MonsterStats.cpp
    src/MonsterStatus.cpp
    src/MonsterTraits.cpp
)
target_include_directories(DDHelperLibrary PUBLIC include)

add_library(
    dearimgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
)
target_include_directories(dearimgui PUBLIC imgui)

add_library(
    imguiapp STATIC
    src/ImguiApp.cpp
    imgui/imgui_impl_sdl.cpp
    imgui/imgui_impl_opengl3.cpp
)
target_include_directories(imguiapp PUBLIC imgui)
target_include_directories(imguiapp PUBLIC /usr/include/SDL2)
target_include_directories(imguiapp PUBLIC include)
target_link_libraries(imguiapp dearimgui)

add_executable(DDHelperTest src/DDHelperTest.cpp)
target_include_directories(DDHelperTest PRIVATE bandit)
target_link_libraries(DDHelperTest DDHelperLibrary)

add_executable(DDHelper src/DDHelper.cpp)
target_link_libraries(DDHelper DDHelperLibrary)
target_link_libraries(DDHelper dearimgui imguiapp)
target_include_directories(DDHelper PUBLIC imgui)
target_include_directories(DDHelper PUBLIC /usr/include/SDL2)
target_link_options(DDHelper PUBLIC "-lGL" "-lGLEW" "-lSDL2")
