#pragma once

class Defence
{
public:
  Defence(int physicalResistPercent = 0,
          int magicalResistPercent = 0,
          int physicalResistPercentMax = 100,
          int magicalResistPercentMax = 100);

  int getPhysicalResistPercent() const;
  int getMagicalResistPercent() const;
  void setPhysicalResistPercent(int physicalResistPercent);
  void setMagicalResistPercent(int magicalResistPercent);

  int getPhysicalResistPercentMax() const;
  int getMagicalResistPercentMax() const;
  void setPhysicalResistPercentMax(int newMax);
  void setMagicalResistPercentMax(int newMax);

  int predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage, int burnStackSize) const;

  void setCorrosion(int numCorrosionLayers);

  void setCursed(bool isCursed);

private:
  int physicalResistPercent;
  int magicalResistPercent;
  int physicalResistPercentMax;
  int magicalResistPercentMax;
  int numCorrosionLayers;
  bool isCursed;
};
