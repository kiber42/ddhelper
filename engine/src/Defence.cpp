#include "engine/Defence.hpp"

#include "engine/MonsterTypes.hpp"

#include <algorithm>

Defence::Defence(MonsterType type)
  : Defence(getPhysicalResistancePercent(type), getMagicalResistancePercent(type), 100, 100)
{
}

Defence::Defence(int physicalResistPercent,
                 int magicalResistPercent,
                 int physicalResistPercentMax,
                 int magicalResistPercentMax)
  : physicalResistPercent(physicalResistPercent)
  , magicalResistPercent(magicalResistPercent)
  , physicalResistPercentMax(physicalResistPercentMax)
  , magicalResistPercentMax(magicalResistPercentMax)
  , numCorrosionLayers(0)
  , numStoneSkinLayers(0)
  , isCursed(false)
{
}

int Defence::getPhysicalResistPercent(bool raw) const
{
  if (raw)
    return physicalResistPercent;
  return std::min(physicalResistPercent + 20 * numStoneSkinLayers, physicalResistPercentMax);
}

int Defence::getMagicalResistPercent(bool raw) const
{
  if (raw)
    return magicalResistPercent;
  return std::min(magicalResistPercent, magicalResistPercentMax);
}

void Defence::setPhysicalResistPercent(int newPhysicalResistPercent)
{
  physicalResistPercent = std::max(newPhysicalResistPercent, 0);
}

void Defence::setMagicalResistPercent(int newMagicalResistPercent)
{
  magicalResistPercent = std::max(newMagicalResistPercent, 0);
}

int Defence::getPhysicalResistPercentMax() const
{
  return physicalResistPercentMax;
}

int Defence::getMagicalResistPercentMax() const
{
  return magicalResistPercentMax;
}

void Defence::setPhysicalResistPercentMax(int newMax)
{
  physicalResistPercentMax = newMax;
  if (physicalResistPercent > physicalResistPercentMax)
    physicalResistPercent = physicalResistPercentMax;
}

void Defence::setMagicalResistPercentMax(int newMax)
{
  magicalResistPercentMax = newMax;
  if (magicalResistPercent > magicalResistPercentMax)
    magicalResistPercent = magicalResistPercentMax;
}

int Defence::predictDamageTaken(int attackerDamageOutput, DamageType damageType, int burnStackSize) const
{
  int damage = attackerDamageOutput + burnStackSize;
  if (damageType == DamageType::Typeless)
    return damage;
  if (!isCursed)
  {
    const int resist = damageType == DamageType::Physical ? getPhysicalResistPercent() : getMagicalResistPercent();
    const int resistedPoints = (attackerDamageOutput * resist + burnStackSize * magicalResistPercent) / 100;
    damage -= resistedPoints;
  }
  if (damage > 0)
    damage += numCorrosionLayers;
  return damage;
}

void Defence::setCorrosion(int corrosion)
{
  numCorrosionLayers = corrosion;
}

void Defence::setStoneSkin(int stoneSkinLayers)
{
  numStoneSkinLayers = stoneSkinLayers;
}

void Defence::setCursed(bool cursed)
{
  isCursed = cursed;
}
