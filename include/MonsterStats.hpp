#pragma once

class MonsterStats
{
public:
  MonsterStats(int level, int hp, int hpMax, int damage, int deathProtection);

  int getLevel() const;

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;

  void healHitPoints(int amountPointsHealed, bool allowOverheal);
  void loseHitPoints(int amountPointsLost);

  int getDamage() const;
  void setDamage(int damagePoints);

  int getDeathProtection() const;
  void setDeathProtection(int numDeathProtectionLayers);

private:
  int level;
  int hp;
  int hpMax;
  int damage;
  int deathProtection;
};
