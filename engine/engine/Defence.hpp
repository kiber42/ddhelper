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
  Defence(int physicalResistPercent = 0,
          int magicalResistPercent = 0,
          int physicalResistPercentMax = 100,
          int magicalResistPercentMax = 100);

  int getPhysicalResistPercent(bool raw = false) const;
  int getMagicalResistPercent(bool raw = false) const;
  void setPhysicalResistPercent(int physicalResistPercent);
  void setMagicalResistPercent(int magicalResistPercent);

  int getPhysicalResistPercentMax() const;
  int getMagicalResistPercentMax() const;
  void setPhysicalResistPercentMax(int newMax);
  void setMagicalResistPercentMax(int newMax);

  int predictDamageTaken(int attackerDamageOutput, DamageType damageType, int burnStackSize) const;

  void setCorrosion(int numCorrosionLayers);
  void setStoneSkin(int stoneSkinLayers);
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
