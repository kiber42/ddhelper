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

add_executable(DDHelperTest src/DDHelperTest.cpp)
target_include_directories(DDHelperTest PRIVATE bandit)
target_link_libraries(DDHelperTest DDHelperLibrary)
