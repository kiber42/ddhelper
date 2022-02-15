#pragma once

class Hero;
#include "engine/DungeonSetup.hpp"
#include "engine/Monster.hpp"

#include <cstdint>
#include <functional>

class Conversion
{
public:
  Conversion(const DungeonSetup&);
  uint8_t getPoints() const;
  uint8_t getThreshold() const;

  // Add points, returns true if threshold was reached
  [[nodiscard]] bool addPoints(unsigned pointsAdded);

  void applyBonus(Hero& hero, Monsters& allMonsters);

private:
  uint8_t points;
  uint8_t threshold;
  std::function<void(Hero&, Monsters&)> bonus;
};
