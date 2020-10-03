#pragma once

#include <optional>
#include <random>

class Hero;

class Jehora
{
public:
  Jehora();
  int initialPietyBonus(int heroLevel);
  int operator()();
  void applyRandomPunishment(Hero& hero);

private:
  std::mt19937 generator;
  int happiness;
  int thresholdPoison;
  int thresholdManaBurn;
  int thresholdHealthLoss;
  int thresholdWeakened;
  int thresholdCorrosion;
  int thresholdCursed;
};
