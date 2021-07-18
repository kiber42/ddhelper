#pragma once

class Hero;
#include "engine/DungeonSetup.hpp"
#include "engine/Monster.hpp"

#include <functional>

class Conversion
{
public:
  Conversion(const DungeonSetup&);
  int getPoints() const;
  int getThreshold() const;

  // Add points, returns true if threshold was reached
  [[nodiscard]] bool addPoints(unsigned pointsAdded);

  void applyBonus(Hero& hero, Monsters& allMonsters);

private:
  int points;
  int threshold;
  std::function<void(Hero&, Monsters&)> bonus;
};
