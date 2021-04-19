#pragma once

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
  HeroStats(int hpMax, int mpMax, int baseDamage);

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;
  void setHitPointsMax(int hitPointsMax);

  int getManaPoints() const;
  int getManaPointsMax() const;
  void setManaPointsMax(int manaPointsMax);

  void healHitPoints(int amountPointsHealed, bool allowOverheal);
  void loseHitPointsWithoutDeathProtection(int amountPointsLost);
  void barelySurvive();

  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);
  void refresh();

  int getBaseDamage() const;
  void setBaseDamage(int damagePoints);

  int getDamageBonusPercent() const;
  void setDamageBonusPercent(int newDamageBonus);

  int getHealthBonus() const;
  // Increase health bonus and also apply it for earlier level-ups
  void addHealthBonus(int unmodifiedLevel);
  // Decrease future health bonuses (do not reduce previous bonuses)
  void reduceHealthBonus();

private:
  int hp;
  int hpMax;
  int mp;
  int mpMax;
  int baseDamage;
  int damageBonusPercent;
  int healthBonus;
};
