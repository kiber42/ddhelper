#include "Defence.hpp"

#include "MonsterTypes.hpp"

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
  , isCursed(false)
{
}

int Defence::getPhysicalResistPercent() const
{
  return physicalResistPercent;
}

int Defence::getMagicalResistPercent() const
{
  return magicalResistPercent;
}

void Defence::setPhysicalResistPercent(int newPhysicalResistPercent)
{
  physicalResistPercent = std::min(std::max(newPhysicalResistPercent, 0), physicalResistPercentMax);
}

void Defence::setMagicalResistPercent(int newMagicalResistPercent)
{
  magicalResistPercent = std::min(std::max(newMagicalResistPercent, 0), magicalResistPercentMax);
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

int Defence::predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage, int burnStackSize) const
{
  int damage = attackerDamageOutput + burnStackSize;
  if (!isCursed)
  {
    const int resist = isMagicalDamage ? magicalResistPercent : physicalResistPercent;
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

void Defence::setCursed(bool cursed)
{
  isCursed = cursed;
}
