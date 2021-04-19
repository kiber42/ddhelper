#pragma once

#include <optional>
#include <random>

class Hero;

class Jehora
{
public:
  Jehora();
  int initialPietyBonus(int heroLevel, bool isPessimist);
  bool lastChanceSuccessful(int remainingPiety);
  int operator()();
  void applyRandomPunishment(Hero& hero);

private:
  std::mt19937 generator{std::random_device{}()};
  int happiness;
  int thresholdPoison;
  int thresholdManaBurn;
  int thresholdHealthLoss;
  int thresholdWeakened;
  int thresholdCorrosion;
  int thresholdCursed;
};
