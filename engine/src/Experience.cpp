#include "engine/Experience.hpp"
#include "engine/StrongTypes.hpp"

#include <algorithm>
#include <cassert>

Experience::Experience(Level initialLevel)
  : level(1)
  , unmodifiedLevel(1)
  , prestige(0)
  , veteran(false)
  , xp(0)
  , xpStep(5)
  , xpNext(5)
{
  while (level < initialLevel)
    gainLevel();
}

Experience::Experience(IsVeteran)
  : level(1)
  , unmodifiedLevel(1)
  , prestige(0)
  , veteran(true)
  , xp(0)
  , xpStep(4)
  , xpNext(4)
{
}

void Experience::gain(ExperiencePoints xpGained)
{
  while (xp + xpGained >= xpNext)
  {
    xpGained -= xpNext - xp;
    gainLevel();
  }
  xp += xpGained;
}

void Experience::gainLevel()
{
  if (level.increase())
    unmodifiedLevel.increase();
  else
    ++prestige;
  xp = 0_xp;
  if (veteran)
  {
    // Increments alternate between +4 and +5
    xpStep = 9_xp - xpStep;
  }
  xpNext += xpStep;
}

void Experience::modifyLevelBy(int delta)
{
  level = Level{level.get() + delta};
}

Level Experience::getUnmodifiedLevel() const
{
  return unmodifiedLevel;
}

ExperiencePoints Experience::forHeroAndMonsterLevels(Level heroLevel, Level monsterLevel)
{
  auto xp = ExperiencePoints{monsterLevel.get()};
  if (monsterLevel > heroLevel)
  {
    const auto delta = monsterLevel.get() - heroLevel.get();
    return xp + ExperiencePoints{delta * (delta - 1) + 2};
  }
  return xp;
}
