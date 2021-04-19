project(ddsolver)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(
  ddsolver STATIC
  src/Scenario.cpp
  src/Solution.cpp
  src/Solver.cpp
  src/SolverTools.cpp
)
target_include_directories(ddsolver PUBLIC .)
target_link_libraries(ddsolver ddhelperengine)
if(NOT WIN32)
  target_link_options(ddsolver PUBLIC "-ltbb")
endif()

add_executable(testsolve src/testsolve.cpp)
target_link_libraries(testsolve ddsolver)
