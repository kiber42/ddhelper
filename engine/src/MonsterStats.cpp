#include "engine/MonsterStats.hpp"

#include "engine/Clamp.hpp"
#include "engine/MonsterTypes.hpp"
#include "engine/StrongTypes.hpp"

#include <algorithm>

namespace
{
  constexpr HitPoints hpInitial(uint8_t level, uint16_t multiplier1, uint16_t multiplier2)
  {
    // TODO: Result sometimes appears to be 1 too high (at least for "Shifting Passages")
    return HitPoints{(level * (level + 6) - 1) * multiplier1 / 100 * multiplier2 / 100};
  }

  constexpr DamagePoints damageInitial(uint8_t level, uint16_t multiplier1, uint16_t multiplier2)
  {
    return DamagePoints{(level * (level + 5) / 2) * multiplier1 / 100 * multiplier2 / 100};
  }
} // namespace

MonsterStats::MonsterStats(MonsterType type, Level level, DungeonMultiplier dungeonMultiplier)
  : type(type)
  , level(level)
  , deathProtection(getDeathProtectionInitial(type, level))
  , dungeonMultiplier(dungeonMultiplier)
  , hp(hpInitial(level.get(), dungeonMultiplier.get(), getHPMultiplierPercent(type)))
  , hpMax(hp)
  , damage(damageInitial(level.get(), dungeonMultiplier.get(), getDamageMultiplierPercent(type)))
{
}

MonsterStats::MonsterStats(Level level, HitPoints hpMax, DamagePoints damage, DeathProtection deathProtection)
  : type(MonsterType::Generic)
  , level(level)
  , deathProtection(deathProtection)
  , dungeonMultiplier(DungeonMultiplier{100})
  , hp(hpMax)
  , hpMax(hp)
  , damage(damage)
{
}

Level MonsterStats::getLevel() const
{
  return level;
}

MonsterType MonsterStats::getType() const
{
  return type;
}

DungeonMultiplier MonsterStats::getDungeonMultiplier() const
{
  return dungeonMultiplier;
}

bool MonsterStats::isDefeated() const
{
  return hp.get() == 0u;
}

HitPoints MonsterStats::getHitPoints() const
{
  return hp;
}

HitPoints MonsterStats::getHitPointsMax() const
{
  return hpMax;
}

void MonsterStats::healHitPoints(HitPoints amountPointsHealed, bool allowOverheal)
{
  hp += amountPointsHealed;
  if (hp > hpMax)
  {
    if (!allowOverheal)
    {
      hp = hpMax;
    }
    else
    {
      const auto overhealMax = hpMax * 3 / 2;
      if (hp > overhealMax)
        hp = overhealMax;
    }
  }
}

void MonsterStats::loseHitPoints(HitPoints amountPointsLost)
{
  if (amountPointsLost < hp)
  {
    hp -= amountPointsLost;
  }
  else
  {
    const auto protection = deathProtection.get();
    if (protection > 0)
    {
      hp = 1_HP;
      deathProtection = DeathProtection{protection - 1};
    }
    else
      hp = 0_HP;
  }
}

void MonsterStats::setHitPointsMax(HitPoints newHitPointsMax)
{
  hpMax = newHitPointsMax > 0_HP ? newHitPointsMax : 1_HP;
}

DamagePoints MonsterStats::getDamage() const
{
  return damage;
}

void MonsterStats::set(DamagePoints damagePoints)
{
  damage = damagePoints;
}

DeathProtection MonsterStats::getDeathProtection() const
{
  return deathProtection;
}

void MonsterStats::set(DeathProtection numDeathProtectionLayers)
{
  deathProtection = numDeathProtectionLayers;
}
