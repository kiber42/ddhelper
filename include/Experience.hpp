#pragma once

#include "ExperienceInterface.hpp"

class Experience : public ExperienceInterface
{
public:
  Experience();

  ExperienceInterface *clone() override { return new Experience(*this); }

  int getXP() const override { return xp; }
  int getLevel() const override { return level; }
  int getPrestige() const override { return prestige; }

  void gain(int xpGained) override;
  void gainLevel() override;
  void modifyLevelBy(int delta) override;

  void setLearning(int learningLevels) override;
  void setExperienceBoost(int active) override;

private:
  int level;
  int prestige;

  int xp;
  int xpNext;

  int learning;
  bool xpBoost;
};
