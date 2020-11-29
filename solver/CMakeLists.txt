project(ddsolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(
    ddsolver STATIC
    src/Solution.cpp
    src/Solver.cpp
)
target_include_directories(ddsolver PUBLIC include)
target_link_libraries(ddsolver ddhelperengine)

add_executable(testsolve src/testsolve.cpp)
target_link_libraries(testsolve ddsolver)
