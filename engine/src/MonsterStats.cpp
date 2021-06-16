#include "engine/MonsterStats.hpp"

#include "engine/MonsterTypes.hpp"

#include <algorithm>

namespace
{
  constexpr int hpInitial(int level, int multiplier1, int multiplier2)
  {
    // TODO: Result sometimes appears to be 1 too high (at least for "Shifting Passages")
    return (level * (level + 6) - 1) * multiplier1 / 100 * multiplier2 / 100;
  }

  constexpr int damageInitial(int level, int multiplier1, int multiplier2)
  {
    return (level * (level + 5) / 2) * multiplier1 / 100 * multiplier2 / 100;
  }
} // namespace

MonsterStats::MonsterStats(MonsterType type, int level, int dungeonMultiplier)
  : type(type)
  , level(level)
  , deathProtection(getDeathProtectionInitial(type, level))
  , dungeonMultiplier(dungeonMultiplier)
  , hp(hpInitial(level, dungeonMultiplier, getHPMultiplierPercent(type)))
  , hpMax(hp)
  , damage(damageInitial(level, dungeonMultiplier, getDamageMultiplierPercent(type)))
{
}

MonsterStats::MonsterStats(int level, int hpMax, int damage, int deathProtection)
  : type(MonsterType::Generic)
  , level(level)
  , deathProtection(deathProtection)
  , dungeonMultiplier(100)
  , hp(hpMax)
  , hpMax(hpMax)
  , damage(damage)
{
}

uint8_t MonsterStats::getLevel() const
{
  return level;
}

MonsterType MonsterStats::getType() const
{
  return type;
}

uint8_t MonsterStats::getDungeonMultiplier() const
{
  return dungeonMultiplier;
}

bool MonsterStats::isDefeated() const
{
  return hp == 0;
}

uint16_t MonsterStats::getHitPoints() const
{
  return hp;
}

uint16_t MonsterStats::getHitPointsMax() const
{
  return hpMax;
}

void MonsterStats::healHitPoints(int amountPointsHealed, bool allowOverheal)
{
  const int max = allowOverheal ? hpMax * 3 / 2 : hpMax;
  if (hp < max)
    hp = std::min(hp + amountPointsHealed, max);
}

void MonsterStats::loseHitPoints(int amountPointsLost)
{
  hp = std::max(hp - amountPointsLost, 0);
  if (hp == 0 && deathProtection > 0)
  {
    hp = 1;
    --deathProtection;
  }
}

void MonsterStats::setHitPointsMax(int newHitPointsMax)
{
  hpMax = std::max(newHitPointsMax, 1);
}

uint16_t MonsterStats::getDamage() const
{
  return damage;
}

void MonsterStats::setDamage(int damagePoints)
{
  damage = damagePoints;
}

uint8_t MonsterStats::getDeathProtection() const
{
  return deathProtection;
}

void MonsterStats::setDeathProtection(int numDeathProtectionLayers)
{
  deathProtection = numDeathProtectionLayers;
}
