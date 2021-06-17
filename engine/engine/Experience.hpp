#pragma once

class Experience
{
public:
  struct IsVeteran
  {
  };

  explicit Experience(unsigned initialLevel = 1);
  Experience(IsVeteran);

  unsigned getXP() const { return xp; }
  unsigned getLevel() const { return level; }
  unsigned getPrestige() const { return prestige; }
  unsigned getXPforNextLevel() const { return xpNext; }

  void gain(unsigned xpGained, unsigned xpBonus, bool xpBoost);
  void gainLevel();
  void modifyLevelBy(int delta);

  // May differ from getLevel if Humility or Blood Curse are involved
  unsigned getUnmodifiedLevel() const;

  static unsigned forHeroAndMonsterLevels(unsigned heroLevel, unsigned monsterLevel);

private:
  unsigned level;
  unsigned unmodifiedLevel;
  unsigned prestige;
  bool veteran;

  unsigned xp;
  unsigned xpStep;
  unsigned xpNext;
};
