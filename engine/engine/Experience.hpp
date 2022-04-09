#pragma once

#include "engine/StrongTypes.hpp"

class Experience
{
public:
  struct IsVeteran
  {
  };

  explicit Experience(Level initialLevel = Level{1});
  Experience(IsVeteran);

  Level getLevel() const { return level; }
  unsigned getPrestige() const { return prestige; }
  ExperiencePoints getXP() const { return xp; }
  ExperiencePoints getXPforNextLevel() const { return xpNext; }

  void gain(ExperiencePoints xpGainedTotal);
  void gainLevel();
  void modifyLevelBy(int delta);

  // May differ from getLevel if Humility or Blood Curse are involved
  Level getUnmodifiedLevel() const;

  static ExperiencePoints forHeroAndMonsterLevels(Level heroLevel, Level monsterLevel);

private:
  Level level;
  Level unmodifiedLevel;
  unsigned prestige;
  bool veteran;

  ExperiencePoints xp;
  ExperiencePoints xpStep;
  ExperiencePoints xpNext;
};
