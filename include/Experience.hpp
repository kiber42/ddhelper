#pragma once

class Experience
{
public:
  struct IsVeteran {};

  explicit Experience(int initialLevel=1);
  Experience(IsVeteran);

  int getXP() const { return xp; }
  int getLevel() const { return level; }
  int getPrestige() const { return prestige; }
  int getXPforNextLevel() const { return xpNext; }

  void gain(int xpGained, int xpBonus, bool xpBoost);
  void gainLevel();
  void modifyLevelBy(int delta);

  static int forHeroAndMonsterLevels(int heroLevel, int monsterLevel);

private:
  int level;
  int prestige;
  bool veteran;

  int xp;
  int xpStep;
  int xpNext;
};
