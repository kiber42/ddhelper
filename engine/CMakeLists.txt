project(ddhelperengine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(
  ddhelperengine STATIC
  src/Combat.cpp
  src/Conversion.cpp
  src/Defence.cpp
  src/Dungeon.cpp
  src/Experience.cpp
  src/Faith.cpp
  src/GlowingGuardian.cpp
  src/Hero.cpp
  src/HeroStats.cpp
  src/Inventory.cpp
  src/Jehora.cpp
  src/Monster.cpp
  src/MonsterStats.cpp
  src/MonsterStatus.cpp
  src/MonsterTraits.cpp
  src/MonsterTypes.cpp
  src/Outcome.cpp
  src/Spells.cpp)
target_include_directories(ddhelperengine PUBLIC include)

add_executable(testengine src/DDHelperTest.cpp)
target_include_directories(testengine PRIVATE bandit)
target_link_libraries(testengine ddhelperengine)
