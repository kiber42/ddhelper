#include "engine/Defence.hpp"

#include "engine/MonsterTypes.hpp"

#include <algorithm>

Defence::Defence(MonsterType type)
  : physicalResist{PhysicalResist{getPhysicalResistancePercent(type)}}
  , magicalResist{MagicalResist{getMagicalResistancePercent(type)}}
{
}

Defence::Defence(PhysicalResist physicalResist)
  : physicalResist{physicalResist}
{
}

Defence::Defence(MagicalResist magicalResist)
  : magicalResist(magicalResist)
{
}

Defence::Defence(PhysicalResist physicalResist,
                 MagicalResist magicalResist,
                 PhysicalResist physicalResistMax,
                 MagicalResist magicalResistMax)
  : physicalResist(physicalResist)
  , magicalResist(magicalResist)
  , physicalResistMax(physicalResistMax)
  , magicalResistMax(magicalResistMax)
{
}

PhysicalResist Defence::getPhysicalResist() const
{
  const unsigned withStoneSkin = physicalResist.in_percent() + 20 * numStoneSkinLayers.get();
  if (withStoneSkin > physicalResistMax.in_percent())
    return physicalResistMax;
  else
    return PhysicalResist{withStoneSkin};
}

MagicalResist Defence::getMagicalResist() const
{
  return std::min(magicalResist, magicalResistMax);
}

void Defence::changePhysicalResistPercent(int deltaPercent)
{
  set(PhysicalResist{getPhysicalResist().in_percent() + deltaPercent});
}

void Defence::changeMagicalResistPercent(int deltaPercent)
{
  set(MagicalResist{getMagicalResist().in_percent() + deltaPercent});
}

void Defence::changePhysicalResistPercentMax(int deltaPercent)
{
  setMax(PhysicalResist{getPhysicalResistMax().in_percent() + deltaPercent});
}

void Defence::changeMagicalResistPercentMax(int deltaPercent)
{
  setMax(MagicalResist{getMagicalResistMax().in_percent() + deltaPercent});
}

DamagePoints
Defence::predictDamageTaken(DamagePoints attackerDamageOutput, DamageType damageType, BurnStackSize burnStackSize) const
{
  auto damage = attackerDamageOutput + DamagePoints{burnStackSize.get()};
  if (damage == 0_damage || damageType == DamageType::Typeless)
    return damage;
  if (!isCursed)
  {
    const auto resistPercent = [&, damageType]() -> unsigned {
      switch (damageType)
      {
      case DamageType::Physical:
        return getPhysicalResist().in_percent();
      case DamageType::Piercing:
      {
        const auto physicalResist = getPhysicalResist();
        const auto pierced = 35_physicalresist;
        if (physicalResist <= pierced)
          return 0;
        if (physicalResist >= 100_physicalresist)
          return 100;
        return (physicalResist - pierced).in_percent();
      }
      case DamageType::Magical:
        return getMagicalResist().in_percent();
      case DamageType::Typeless:
        return 0;
      }
    }();
    const auto resistedPoints =
        (attackerDamageOutput * resistPercent + DamagePoints{burnStackSize.get() * getMagicalResist().in_percent()}) /
        100;
    damage -= resistedPoints;
  }
  if (damage > 0_damage)
    damage += DamagePoints{numCorrosionLayers.get()};
  return damage;
}

void Defence::set(CorrosionAmount corrosionLayers)
{
  numCorrosionLayers = corrosionLayers;
}

void Defence::set(StoneSkinLayers stoneSkinLayers)
{
  numStoneSkinLayers = stoneSkinLayers;
}

void Defence::setCursed(bool cursed)
{
  isCursed = cursed;
}
