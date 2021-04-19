#pragma once

class Hero;
#include "engine/HeroClass.hpp"
#include "engine/Monster.hpp"

#include <functional>

class Conversion
{
public:
  Conversion(HeroClass theClass, HeroRace race);
  int getPoints() const;
  int getThreshold() const;

  // Add points, returns true if threshold was reached
  [[nodiscard]] bool addPoints(int pointsAdded);

  void applyBonus(Hero& hero, Monsters& allMonsters);

private:
  int points;
  int threshold;
  std::function<void(Hero&, Monsters&)> bonus;
};
