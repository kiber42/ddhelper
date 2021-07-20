#pragma once

#include "StrongTypes.hpp"

#include <cstdint>

class HeroStats
{
public:
  struct IsDangerous
  {
  };
  struct RegalSize
  {
  };

  HeroStats();
  HeroStats(IsDangerous);
  HeroStats(RegalSize);
  HeroStats(HitPoints hpMax, ManaPoints mpMax, DamagePoints baseDamage);

  bool isDefeated() const;
  HitPoints getHitPoints() const;
  HitPoints getHitPointsMax() const;
  void setHitPointsMax(HitPoints hitPointsMax);

  ManaPoints getManaPoints() const;
  ManaPoints getManaPointsMax() const;
  void setManaPointsMax(ManaPoints manaPointsMax);

  void healHitPoints(HitPoints amountPointsHealed, bool allowOverheal);
  void loseHitPointsWithoutDeathProtection(HitPoints amountPointsLost);
  void barelySurvive();

  void recoverManaPoints(ManaPoints amountPointsRecovered);
  void loseManaPoints(ManaPoints amountPointsLost);
  void refresh();

  DamagePoints getBaseDamage() const;
  void setBaseDamage(DamagePoints damagePoints);

  DamageBonus getDamageBonus() const;
  void setDamageBonus(DamageBonus newDamageBonus);

  int getHealthBonus() const;
  // Increase health bonus and also apply it for earlier level-ups
  void addHealthBonus(Level heroLevel);
  // Increase health bonus, but do not apply it retroactively
  void addFutureHealthBonus();
  // Decrease future health bonuses (do not reduce previous bonuses)
  void reduceHealthBonus();

private:
  HitPoints hp;
  HitPoints hpMax;
  ManaPoints mp;
  ManaPoints mpMax;
  DamagePoints baseDamage;
  DamageBonus damageBonusPercent;
  int8_t healthBonus;
};
