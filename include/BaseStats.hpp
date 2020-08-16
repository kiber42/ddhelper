#pragma once

class BaseStats
{
public:
  BaseStats(int level, int hpMax, int mpMax, int damage);

  int getLevel() const;
  void setLevel(int level);

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;
  void setHitPointsMax(int hitPointsMax, bool capHitPoints);
  int getManaPoints() const;
  int getManaPointsMax() const;
  void setManaPointsMax(int manaPointsMax);

  int getDamage() const;
  void setDamage(int damagePoints);

  void healHitPoints(int amountPointsHealed, bool allowOverheal);
  void loseHitPoints(int amountPointsLost);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);
  void refresh();

  int getDeathProtection() const;
  void setDeathProtection(int numDeathProtectionLayers);

private:
  int level;
  int hp;
  int hpMax;
  int mp;
  int mpMax;
  int damage;
  int deathProtection;
};
