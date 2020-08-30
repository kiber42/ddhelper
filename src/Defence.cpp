#include "Defence.hpp"

#include <algorithm>

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

int Defence::predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const
{
  int damage = attackerDamageOutput;
  if (!isCursed)
  {
    const int resist = isMagicalDamage ? magicalResistPercent : physicalResistPercent;
    const int resistedPoints = attackerDamageOutput * resist / 100;
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
