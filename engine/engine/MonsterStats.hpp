#pragma once

#include "StrongTypes.hpp"

#include <cstdint>

enum class MonsterType : uint8_t;

class MonsterStats
{
public:
  MonsterStats(MonsterType, Level, DungeonMultiplier);
  MonsterStats(Level, HitPoints, DamagePoints, DeathProtection = DeathProtection{0});

  MonsterType getType() const;
  Level getLevel() const;
  DungeonMultiplier getDungeonMultiplier() const;

  bool isDefeated() const;
  HitPoints getHitPoints() const;
  HitPoints getHitPointsMax() const;

  void healHitPoints(HitPoints amountPointsHealed, bool allowOverheal);
  void loseHitPoints(HitPoints amountPointsLost);
  void setHitPointsMax(HitPoints newHitPointsMax);
  //! Set HP, capped at 150% HP max
  void setHitPoints(HitPoints newHitPoints);

  DamagePoints getDamage() const;
  void set(DamagePoints);

  DeathProtection getDeathProtection() const;
  void set(DeathProtection);

private:
  MonsterType type;
  Level level;
  DeathProtection deathProtection;
  DungeonMultiplier dungeonMultiplier;
  HitPoints hp;
  HitPoints hpMax;
  DamagePoints damage;
};
