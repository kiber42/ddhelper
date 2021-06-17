#pragma once

#include <optional>
#include <random>

class Hero;

class Jehora
{
public:
  Jehora();
  unsigned initialPietyBonus(unsigned heroLevel, bool isPessimist);
  bool lastChanceSuccessful(int remainingPiety);
  unsigned operator()();
  void applyRandomPunishment(Hero& hero);

private:
  std::mt19937 generator{std::random_device{}()};
  unsigned happiness;
  unsigned thresholdPoison;
  unsigned thresholdManaBurn;
  unsigned thresholdHealthLoss;
  unsigned thresholdWeakened;
  unsigned thresholdCorrosion;
  unsigned thresholdCursed;
};
