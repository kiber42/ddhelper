#pragma once

class Defence
{
public:
  Defence(int physicalResistPercent = 0, int magicalResistPercent = 0);

  int getPhysicalResistPercent() const;
  int getMagicalResistPercent() const;
  void setPhysicalResistPercent(int physicalResistPercent);
  void setMagicalResistPercent(int magicalResistPercent);

  int predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const;

  void setCorrosion(int numCorrosionLayers);

private:
  int physicalResistPercent;
  int magicalResistPercent;
  int numCorrosionLayers;
};
