#include "Defence.hpp"

#include <algorithm>

Defence::Defence(int physicalResistPercent, int magicalResistPercent)
  : physicalResistPercent(physicalResistPercent)
  , magicalResistPercent(magicalResistPercent)
  , numCorrosionLayers(0)
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
  physicalResistPercent = std::min(std::max(newPhysicalResistPercent, 0), 100);
}

void Defence::setMagicalResistPercent(int newMagicalResistPercent)
{
  magicalResistPercent = std::min(std::max(newMagicalResistPercent, 0), 100);
}

int Defence::predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const
{
  const int resist = isMagicalDamage ? magicalResistPercent : physicalResistPercent;
  const int resistedPoints = attackerDamageOutput * resist / 100;
  int damage = attackerDamageOutput - resistedPoints;
  if (damage > 0)
    damage += numCorrosionLayers;
  return damage;
}

void Defence::setCorrosion(int corrosion)
{
  numCorrosionLayers = corrosion;
}
