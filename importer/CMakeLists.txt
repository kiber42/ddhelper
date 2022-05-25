project(ddimporter VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OpenCV has several conversion issues
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_compile_options(-Wno-sign-conversion)
  add_compile_options(-Wno-deprecated-enum-enum-conversion)
  add_compile_options(-Wno-deprecated-anon-enum-enum-conversion)
  add_compile_options(-Wno-implicit-float-conversion)
  add_compile_options(-Wno-float-conversion)
endif()

add_library(
  ddimporter STATIC
  src/ImageCapture.cpp
  src/ImageProcessor.cpp
  src/ImportedState.cpp
  src/GameWindow.cpp
  src/Mouse.cpp
)
target_include_directories(ddimporter PUBLIC .)
target_link_libraries(ddimporter PUBLIC ddhelperengine)
if(NOT WIN32)
  target_include_directories(ddimporter PUBLIC /usr/include/opencv4)
  target_link_libraries(ddimporter PUBLIC opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs X11 Xext)
endif()

add_executable(
  importer
  src/importer.cpp)
target_include_directories(importer PUBLIC .)
target_link_libraries(importer ddimporter)
