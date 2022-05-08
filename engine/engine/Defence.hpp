#pragma once

#include "engine/Clamp.hpp"
#include "engine/StrongTypes.hpp"

#include <cstdint>

enum class MonsterType : uint8_t;

enum class DamageType : uint8_t
{
  Physical,
  Piercing,
  Magical,
  Typeless
};

enum class Speed : uint8_t
{
  Slow,
  Normal,
  Fast,
  SuperFast,
};

using PhysicalResist = Percentage<uint8_t, struct PhysicalResistParam, Addable, Subtractable, Comparable>;
using MagicalResist = Percentage<uint8_t, struct MagicalResistParam, Addable, Subtractable, Comparable>;

auto constexpr operator"" _physicalresist(unsigned long long value)
{
  return PhysicalResist{clampedTo<uint8_t>(value)};
}
auto constexpr operator"" _magicalresist(unsigned long long value)
{
  return MagicalResist{clampedTo<uint8_t>(value)};
}

class Defence
{
public:
  Defence(MonsterType type);

  Defence() = default;
  Defence(PhysicalResist);
  Defence(MagicalResist);

  Defence(PhysicalResist physicalResist,
          MagicalResist magicalResist,
          PhysicalResist physicalResistMax = 100_physicalresist,
          MagicalResist magicalResistMax = 100_magicalresist);

  // Retrieve effective resist, accounting for maximum and stone skin modifier
  PhysicalResist getPhysicalResist() const;
  MagicalResist getMagicalResist() const;

  // Retrieve resist without capping at maximum or applying stone skin modifier
  PhysicalResist getPhysicalResistRaw() const { return physicalResist; }
  MagicalResist getMagicalResistRaw() const { return magicalResist; }

  PhysicalResist getPhysicalResistMax() const { return physicalResistMax; }
  MagicalResist getMagicalResistMax() const { return magicalResistMax; }

  void set(PhysicalResist newResist) { physicalResist = newResist; }
  void set(MagicalResist newResist) { magicalResist = newResist; }
  void setMax(PhysicalResist newMax) { physicalResistMax = newMax; }
  void setMax(MagicalResist newMax) { magicalResistMax = newMax; }

  void changePhysicalResistPercent(int deltaPercent);
  void changeMagicalResistPercent(int deltaPercent);
  void changePhysicalResistPercentMax(int deltaPercent);
  void changeMagicalResistPercentMax(int deltaPercent);

  DamagePoints predictDamageTaken(DamagePoints attackerDamageOutput, DamageType, BurnStackSize) const;

  void set(CorrosionAmount);
  void set(StoneSkinLayers);
  void setCursed(bool isCursed);

private:
  PhysicalResist physicalResist{0_physicalresist};
  MagicalResist magicalResist{0_magicalresist};
  PhysicalResist physicalResistMax{100_physicalresist};
  MagicalResist magicalResistMax{100_magicalresist};
  CorrosionAmount numCorrosionLayers{0_corrosion};
  StoneSkinLayers numStoneSkinLayers{0_stoneskin};
  bool isCursed{false};
};
