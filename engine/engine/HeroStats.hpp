#pragma once

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
  HeroStats(uint16_t hpMax, uint16_t mpMax, uint16_t baseDamage);

  bool isDefeated() const;
  uint16_t getHitPoints() const;
  uint16_t getHitPointsMax() const;
  void setHitPointsMax(uint16_t hitPointsMax);

  uint16_t getManaPoints() const;
  uint16_t getManaPointsMax() const;
  void setManaPointsMax(uint16_t manaPointsMax);

  void healHitPoints(unsigned amountPointsHealed, bool allowOverheal);
  void loseHitPointsWithoutDeathProtection(uint16_t amountPointsLost);
  void barelySurvive();

  void recoverManaPoints(uint16_t amountPointsRecovered);
  void loseManaPoints(uint16_t amountPointsLost);
  void refresh();

  uint16_t getBaseDamage() const;
  void setBaseDamage(uint16_t damagePoints);

  int16_t getDamageBonusPercent() const;
  void setDamageBonusPercent(int16_t newDamageBonus);

  int getHealthBonus() const;
  // Increase health bonus and also apply it for earlier level-ups
  void addHealthBonus(uint8_t unmodifiedLevel);
  // Decrease future health bonuses (do not reduce previous bonuses)
  void reduceHealthBonus();

private:
  uint16_t hp;
  uint16_t hpMax;
  uint16_t mp;
  uint16_t mpMax;
  uint16_t baseDamage;
  int16_t damageBonusPercent;
  int8_t healthBonus;
};
