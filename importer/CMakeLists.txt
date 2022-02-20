project(ddimporter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OpenCV has several conversion issues
add_compile_options(-Wno-sign-conversion)
add_compile_options(-Wno-deprecated-enum-enum-conversion)
add_compile_options(-Wno-deprecated-anon-enum-enum-conversion)
add_compile_options(-Wno-implicit-float-conversion)
add_compile_options(-Wno-float-conversion)

add_library(
  ddimporter STATIC
  src/ImageCapture.cpp
  src/GameWindow.cpp
  src/Mouse.cpp
)
target_include_directories(ddimporter PUBLIC .)
target_include_directories(ddimporter PUBLIC /usr/include/opencv4)
target_link_libraries(ddimporter PUBLIC "-lopencv_core" "-lopencv_highgui" "-lopencv_imgproc" "-lopencv_imgcodecs" "-lX11" "-lXext")

add_executable(
  importer
  src/importer.cpp)
target_include_directories(importer PUBLIC .)
target_link_libraries(importer ddimporter)
target_link_libraries(importer ddhelperengine)

add_executable(
  captureTest
  src/captureTest.cpp)

target_include_directories(captureTest PUBLIC .)
target_link_libraries(captureTest ddimporter)
