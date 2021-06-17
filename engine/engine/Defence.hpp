#pragma once

#include <cstdint>

enum class MonsterType : uint8_t;

enum class DamageType : uint8_t
{
  Physical,
  Piercing,
  Magical,
  Typeless
};

class Defence
{
public:
  Defence(MonsterType type);
  Defence(uint8_t physicalResistPercent = 0,
          uint8_t magicalResistPercent = 0,
          uint8_t physicalResistPercentMax = 100,
          uint8_t magicalResistPercentMax = 100);

  uint8_t getPhysicalResistPercent(bool raw = false) const;
  uint8_t getMagicalResistPercent(bool raw = false) const;
  void setPhysicalResistPercent(uint8_t physicalResistPercent);
  void setMagicalResistPercent(uint8_t magicalResistPercent);

  uint8_t getPhysicalResistPercentMax() const;
  uint8_t getMagicalResistPercentMax() const;
  void setPhysicalResistPercentMax(uint8_t newMax);
  void setMagicalResistPercentMax(uint8_t newMax);

  unsigned predictDamageTaken(unsigned attackerDamageOutput, DamageType damageType, uint8_t burnStackSize) const;

  void setCorrosion(uint8_t numCorrosionLayers);
  void setStoneSkin(uint8_t stoneSkinLayers);
  void setCursed(bool isCursed);

private:
  uint8_t physicalResistPercent;
  uint8_t magicalResistPercent;
  uint8_t physicalResistPercentMax;
  uint8_t magicalResistPercentMax;
  uint8_t numCorrosionLayers;
  uint8_t numStoneSkinLayers;
  bool isCursed;
};
