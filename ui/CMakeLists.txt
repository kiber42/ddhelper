project(ddhelperui VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
  ddhelper
  src/Arena.cpp
  src/DDHelper.cpp
  src/HeroSelection.cpp
  src/History.cpp
  src/ImguiAppOpenGL.cpp
  src/MonsterPool.cpp
  src/MonsterSelection.cpp
  src/Resources.cpp
  src/RunHeuristics.cpp
  src/RunImporter.cpp
  src/RunSolver.cpp
  src/State.cpp
  src/Utils.cpp)
target_link_libraries(ddhelper ddhelperengine ddsolver ddimporter dearimgui)
target_include_directories(ddhelper PUBLIC .)
