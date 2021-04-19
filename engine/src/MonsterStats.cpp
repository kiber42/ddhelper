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
  , dungeonMultiplier(dungeonMultiplier)
  , hp(hpInitial(level, dungeonMultiplier, getHPMultiplierPercent(type)))
  , hpMax(hp)
  , damage(damageInitial(level, dungeonMultiplier, getDamageMultiplierPercent(type)))
  , deathProtection(getDeathProtectionInitial(type, level))
{
}

MonsterStats::MonsterStats(int level, int hpMax, int damage, int deathProtection)
  : type(MonsterType::Generic)
  , level(level)
  , hp(hpMax)
  , hpMax(hpMax)
  , damage(damage)
  , deathProtection(deathProtection)
{
}

int MonsterStats::getLevel() const
{
  return level;
}

MonsterType MonsterStats::getType() const
{
  return type;
}

int MonsterStats::getDungeonMultiplier() const
{
  return dungeonMultiplier;
}

bool MonsterStats::isDefeated() const
{
  return hp == 0;
}

int MonsterStats::getHitPoints() const
{
  return hp;
}

int MonsterStats::getHitPointsMax() const
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

int MonsterStats::getDamage() const
{
  return damage;
}

void MonsterStats::setDamage(int damagePoints)
{
  damage = damagePoints;
}

int MonsterStats::getDeathProtection() const
{
  return deathProtection;
}

void MonsterStats::setDeathProtection(int numDeathProtectionLayers)
{
  deathProtection = numDeathProtectionLayers;
}
