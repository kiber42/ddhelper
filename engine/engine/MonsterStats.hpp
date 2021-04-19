#pragma once

enum class MonsterType;

class MonsterStats
{
public:
  MonsterStats(MonsterType type, int level, int dungeonMultiplier);
  MonsterStats(int level, int hpMax, int damage, int deathProtection);

  MonsterType getType() const;
  int getLevel() const;
  int getDungeonMultiplier() const;

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;

  void healHitPoints(int amountPointsHealed, bool allowOverheal);
  void loseHitPoints(int amountPointsLost);
  void setHitPointsMax(int newHitPointsMax);

  int getDamage() const;
  void setDamage(int damagePoints);

  int getDeathProtection() const;
  void setDeathProtection(int numDeathProtectionLayers);

private:
  MonsterType type;
  int level;
  int dungeonMultiplier;
  int hp;
  int hpMax;
  int damage;
  int deathProtection;
};
