project(ddbridge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OpenCV has several conversion issues
add_compile_options(-Wno-sign-conversion)
add_compile_options(-Wno-deprecated-enum-enum-conversion)
add_compile_options(-Wno-deprecated-anon-enum-enum-conversion)
add_compile_options(-Wno-implicit-float-conversion)
add_compile_options(-Wno-float-conversion)

add_library(
  ddbridge STATIC
  src/capture.cpp
)
target_include_directories(ddbridge PUBLIC .)
target_include_directories(ddbridge PUBLIC /usr/include/opencv4)
target_link_libraries(ddbridge PUBLIC "-lopencv_core" "-lopencv_highgui" "-lX11" "-lXext")

add_executable(
  bridge
  src/bridge.cpp)
target_link_libraries(bridge ddbridge)
