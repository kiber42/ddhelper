#pragma once

class ExperienceInterface
{
public:
  virtual ~ExperienceInterface() = default;
  virtual ExperienceInterface *clone() = 0;

  virtual int getXP() const = 0;
  virtual int getLevel() const = 0;
  virtual int getPrestige() const = 0;

  virtual void gain(int xpGained) = 0;
  virtual void gainLevel() = 0;
  virtual void modifyLevelBy(int delta) = 0;

  virtual void setLearning(int learningLevels) = 0;
  virtual void setExperienceBoost(int active) = 0;
};
