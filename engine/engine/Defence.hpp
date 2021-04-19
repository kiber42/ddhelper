#pragma once

enum class MonsterType;

enum class DamageType
{
  Physical,
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
  int physicalResistPercent;
  int magicalResistPercent;
  int physicalResistPercentMax;
  int magicalResistPercentMax;
  int numCorrosionLayers;
  int numStoneSkinLayers;
  bool isCursed;
};
