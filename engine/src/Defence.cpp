#include "engine/Defence.hpp"

#include "engine/MonsterTypes.hpp"

#include <algorithm>

Defence::Defence(MonsterType type)
  : Defence(getPhysicalResistancePercent(type), getMagicalResistancePercent(type), 100, 100)
{
}

Defence::Defence(uint8_t physicalResistPercent,
                 uint8_t magicalResistPercent,
                 uint8_t physicalResistPercentMax,
                 uint8_t magicalResistPercentMax)
  : physicalResistPercent(physicalResistPercent)
  , magicalResistPercent(magicalResistPercent)
  , physicalResistPercentMax(physicalResistPercentMax)
  , magicalResistPercentMax(magicalResistPercentMax)
  , numCorrosionLayers(0)
  , numStoneSkinLayers(0)
  , isCursed(false)
{
}

uint8_t Defence::getPhysicalResistPercent(bool raw) const
{
  if (raw)
    return physicalResistPercent;
  return std::min(physicalResistPercent + 20 * numStoneSkinLayers, static_cast<int>(physicalResistPercentMax));
}

uint8_t Defence::getMagicalResistPercent(bool raw) const
{
  if (raw)
    return magicalResistPercent;
  return std::min(magicalResistPercent, magicalResistPercentMax);
}

void Defence::setPhysicalResistPercent(uint8_t newPhysicalResistPercent)
{
  physicalResistPercent = newPhysicalResistPercent;
}

void Defence::setMagicalResistPercent(uint8_t newMagicalResistPercent)
{
  magicalResistPercent = newMagicalResistPercent;
}

uint8_t Defence::getPhysicalResistPercentMax() const
{
  return physicalResistPercentMax;
}

uint8_t Defence::getMagicalResistPercentMax() const
{
  return magicalResistPercentMax;
}

void Defence::setPhysicalResistPercentMax(uint8_t newMax)
{
  physicalResistPercentMax = newMax;
  if (physicalResistPercent > physicalResistPercentMax)
    physicalResistPercent = physicalResistPercentMax;
}

void Defence::setMagicalResistPercentMax(uint8_t newMax)
{
  magicalResistPercentMax = newMax;
  if (magicalResistPercent > magicalResistPercentMax)
    magicalResistPercent = magicalResistPercentMax;
}

int Defence::predictDamageTaken(int attackerDamageOutput, DamageType damageType, uint8_t burnStackSize) const
{
  int damage = attackerDamageOutput + burnStackSize;
  if (damageType == DamageType::Typeless)
    return damage;
  if (!isCursed)
  {
    const int resist = [&, damageType]() -> int {
      switch (damageType)
      {
      case DamageType::Physical:
        return getPhysicalResistPercent();
      case DamageType::Piercing:
        return std::max(getPhysicalResistPercent() - 35, 0);
      case DamageType::Magical:
        return getMagicalResistPercent();
      case DamageType::Typeless:
        return 0;
      }
    }();
    const int resistedPoints = (attackerDamageOutput * resist + burnStackSize * magicalResistPercent) / 100;
    damage -= resistedPoints;
  }
  if (damage > 0)
    damage += numCorrosionLayers;
  return damage;
}

void Defence::setCorrosion(uint8_t corrosion)
{
  numCorrosionLayers = corrosion;
}

void Defence::setStoneSkin(uint8_t stoneSkinLayers)
{
  numStoneSkinLayers = stoneSkinLayers;
}

void Defence::setCursed(bool cursed)
{
  isCursed = cursed;
}
