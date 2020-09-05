#pragma once

class Experience
{
public:
  explicit Experience(bool hasVeteranTrait=false);

  int getXP() const { return xp; }
  int getLevel() const { return level; }
  int getPrestige() const { return prestige; }

  void gain(int xpGained, int xpBonus, bool xpBoost);
  void gainLevel();
  void modifyLevelBy(int delta);

private:
  int level;
  int prestige;
  bool veteran;

  int xp;
  int xpStep;
  int xpNext;
};
