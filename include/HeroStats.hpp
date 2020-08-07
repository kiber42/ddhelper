#pragma once

#include "BaseStats.hpp"

class HeroStats
{
public:
  HeroStats(int level, int hpMax, int mpMax);

  int getLevel() const;
  void setLevel(int level);

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;
  void setHitPointsMax(int hitPointsMax);
  int getManaPoints() const;
  int getManaPointsMax() const;
  void setManaPointsMax(int manaPointsMax);

  int getHealthBonus() const;
  void addHealthBonus();

  void healHitPoints(int amountPointsHealed, bool allowOverheal);
  void loseHitPoints(int amountPointsLost);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);
  void refresh();

  int getDeathProtection() const;
  void setDeathProtection(bool enableProtection);

private:
  BaseStats stats;
  int healthBonus;
};
