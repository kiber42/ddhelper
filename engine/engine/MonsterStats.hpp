#pragma once

#include <cstdint>

enum class MonsterType : uint8_t;

class MonsterStats
{
public:
  MonsterStats(MonsterType type, uint8_t level, uint8_t dungeonMultiplier);
  MonsterStats(uint8_t level, uint16_t hpMax, uint16_t damage, uint8_t deathProtection);

  MonsterType getType() const;
  uint8_t getLevel() const;
  uint8_t getDungeonMultiplier() const;

  bool isDefeated() const;
  uint16_t getHitPoints() const;
  uint16_t getHitPointsMax() const;

  void healHitPoints(uint16_t amountPointsHealed, bool allowOverheal);
  void loseHitPoints(uint16_t amountPointsLost);
  void setHitPointsMax(uint16_t newHitPointsMax);

  uint16_t getDamage() const;
  void setDamage(uint16_t damagePoints);

  uint8_t getDeathProtection() const;
  void setDeathProtection(uint8_t numDeathProtectionLayers);

private:
  MonsterType type;
  uint8_t level;
  uint8_t deathProtection;
  uint8_t dungeonMultiplier;
  uint16_t hp;
  uint16_t hpMax;
  uint16_t damage;
};
