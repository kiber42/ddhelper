#pragma once

#include "BaseStats.hpp"

class MonsterStats
{
public:
  MonsterStats(int level, int hp, int hpMax, int damage, int deathProtection);

  int getLevel() const;

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;

  int getDamage() const;
  void setDamage(int damagePoints);

  void healHitPoints(int amountPointsHealed, bool allowOverheal);
  void loseHitPoints(int amountPointsLost);

  int getDeathProtection() const;
  void setDeathProtection(int numDeathProtectionLayers);

private:
  BaseStats stats;
};
