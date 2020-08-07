#include "Experience.hpp"

#include <algorithm>
#include <cassert>

Experience::Experience()
  : level(1)
  , prestige(0)
  , xp(0)
  , xpNext(5)
  , learning(0)
  , xpBoost(false)
{
}

void Experience::gain(int xpGained)
{
  if (xpBoost)
    xpGained += xpGained / 2;

  xpGained += learning;

  while (xp + xpGained >= xpNext)
  {
    xpGained -= xpNext - xp;
    gainLevel();
  }
  xp += xpGained;
}

void Experience::gainLevel()
{
  if (level < 10)
    ++level;
  else
    ++prestige;
  xpNext += 5;
  xp = 0;
}

void Experience::modifyLevelBy(int delta)
{
  level = std::min(std::max(level + delta, 1), 10);
}

void Experience::setLearning(int learningLevels)
{
  learning = learningLevels;
}

void Experience::setExperienceBoost(int active)
{
  xpBoost = active > 0;
}
