#pragma once

class HeroStats
{
public:
  HeroStats();
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
  void addHealthBonus(int unmodifiedLevel);

private:
  int hp;
  int hpMax;
  int mp;
  int mpMax;
  int baseDamage;
  int damageBonusPercent;
  int healthBonus;
};
