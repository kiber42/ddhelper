project(ddimporter VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (WIN32)
  get_filename_component(OpenCV_DIR "../opencv" ABSOLUTE .)
endif()
find_package(OpenCV REQUIRED)

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
target_include_directories(ddimporter PUBLIC . ${OpenCV_INCLUDE_DIRS})
target_link_libraries(ddimporter PUBLIC ddhelperengine opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs)
if(NOT WIN32)
  target_link_libraries(ddimporter PUBLIC X11 Xext)
endif()

add_executable(
  importer
  src/importer.cpp)
target_link_libraries(importer ddimporter)

get_filename_component(resourceDirUnix ${CMAKE_CURRENT_SOURCE_DIR}/testimages ABSOLUTE)
file(TO_NATIVE_PATH ${resourceDirUnix}/ resourceDir)
configure_file(importer/testimporter.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/importer/testimporter.hpp)

add_executable(
  testimporter
  src/testimporter.cpp)
target_include_directories(testimporter PRIVATE ../bandit)
target_link_libraries(testimporter ddimporter)
