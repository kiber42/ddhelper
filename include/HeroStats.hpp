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
  void loseHitPoints(int amountPointsLost);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);
  void refresh();

  int getBaseDamage() const;
  void setBaseDamage(int damagePoints);

  int getDamageBonusPercent() const;
  void setDamageBonusPercent(int newDamageBonus);

  int getHealthBonus() const;
  void addHealthBonus(int unmodifiedLevel);

  bool getDeathProtection() const;
  void setDeathProtection(bool enableProtection);

private:
  int hp;
  int hpMax;
  int mp;
  int mpMax;
  int baseDamage;
  int damageBonusPercent;
  int healthBonus;
  bool deathProtection;
};
