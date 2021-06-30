project(ddhelperengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(
  ddhelperengine STATIC
  src/Combat.cpp
  src/Conversion.cpp
  src/Defence.cpp
  src/Dungeon.cpp
  src/DungeonSetup.cpp
  src/Experience.cpp
  src/Faith.cpp
  src/GlowingGuardian.cpp
  src/Hero.cpp
  src/HeroStats.cpp
  src/Inventory.cpp
  src/Jehora.cpp
  src/Magic.cpp
  src/Monster.cpp
  src/MonsterStats.cpp
  src/MonsterTraits.cpp
  src/Outcome.cpp
  src/PietyChange.cpp
  src/Resources.cpp)
target_include_directories(ddhelperengine PUBLIC .)

add_executable(
  testengine
  test/main.cpp
  test/testCombat.cpp
  test/testDefence.cpp
  test/testDungeon.cpp
  test/testExperience.cpp
  test/testFaith.cpp
  test/testInventory.cpp
  test/testMonsters.cpp
  test/testPotions.cpp
  test/testResources.cpp
  test/testSpells.cpp
  test/testStatusEffects.cpp
  test/testTraits.cpp)
target_include_directories(testengine PRIVATE bandit)
target_link_libraries(testengine ddhelperengine)
