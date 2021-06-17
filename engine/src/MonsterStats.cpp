#include "engine/MonsterStats.hpp"

#include "engine/Clamp.hpp"
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

MonsterStats::MonsterStats(MonsterType type, uint8_t level, uint8_t dungeonMultiplier)
  : type(type)
  , level(clamped<uint8_t>(level, 1, 10))
  , deathProtection(getDeathProtectionInitial(type, level))
  , dungeonMultiplier(clampedTo<uint8_t>(dungeonMultiplier))
  , hp(hpInitial(level, dungeonMultiplier, getHPMultiplierPercent(type)))
  , hpMax(hp)
  , damage(damageInitial(level, dungeonMultiplier, getDamageMultiplierPercent(type)))
{
}

MonsterStats::MonsterStats(uint8_t level, uint16_t hpMax, uint16_t damage, uint8_t deathProtection)
  : type(MonsterType::Generic)
  , level(clamped<uint8_t>(level, 1, 10))
  , deathProtection(deathProtection)
  , dungeonMultiplier(100)
  , hp(hpMax)
  , hpMax(hp)
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

void MonsterStats::healHitPoints(uint16_t amountPointsHealed, bool allowOverheal)
{
  const auto max = allowOverheal ? hpMax * 3 / 2 : hpMax;
  if (hp < max)
    hp = std::min(hp + amountPointsHealed, max);
}

void MonsterStats::loseHitPoints(uint16_t amountPointsLost)
{
  if (amountPointsLost < hp)
  {
    hp -= amountPointsLost;
  }
  else
  {
    if (deathProtection > 0)
    {
      hp = 1;
      --deathProtection;
    }
    else
      hp = 0;
  }
}

void MonsterStats::setHitPointsMax(uint16_t newHitPointsMax)
{
  hpMax = newHitPointsMax > 0 ? newHitPointsMax : 1;
}

uint16_t MonsterStats::getDamage() const
{
  return damage;
}

void MonsterStats::setDamage(uint16_t damagePoints)
{
  damage = damagePoints;
}

uint8_t MonsterStats::getDeathProtection() const
{
  return deathProtection;
}

void MonsterStats::setDeathProtection(uint8_t numDeathProtectionLayers)
{
  deathProtection = numDeathProtectionLayers;
}
