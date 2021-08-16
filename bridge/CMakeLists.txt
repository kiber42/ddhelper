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
  src/ImageCapture.cpp
  src/GameWindow.cpp
  src/Mouse.cpp
)
target_include_directories(ddbridge PUBLIC .)
target_include_directories(ddbridge PUBLIC /usr/include/opencv4)
target_link_libraries(ddbridge PUBLIC "-lopencv_core" "-lopencv_highgui" "-lopencv_imgproc" "-lopencv_imgcodecs" "-lX11" "-lXext")

add_executable(
  bridge
  src/bridge.cpp)
target_include_directories(bridge PUBLIC .)
target_link_libraries(bridge ddbridge)
target_link_libraries(bridge ddhelperengine)

add_executable(
  captureTest
  src/captureTest.cpp)

target_include_directories(captureTest PUBLIC .)
target_link_libraries(captureTest ddbridge)
