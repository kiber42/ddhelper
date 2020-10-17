project(ddhelperui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

add_executable(
    ddhelper
    src/Arena.cpp
    src/DDHelper.cpp
    src/HeroSelection.cpp
    src/History.cpp
    src/MonsterPool.cpp
    src/MonsterSelection.cpp
    src/Scenario.cpp
    src/Utils.cpp
)
target_link_libraries(ddhelper ddhelperengine imguiapp)
target_include_directories(ddhelper PUBLIC imgui)
target_include_directories(ddhelper PUBLIC /usr/include/SDL2)
