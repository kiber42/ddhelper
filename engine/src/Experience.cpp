#include "engine/Experience.hpp"

#include <algorithm>
#include <cassert>

Experience::Experience(unsigned initialLevel)
  : level(1)
  , unmodifiedLevel(1)
  , prestige(0)
  , veteran(false)
  , xp(0)
  , xpStep(5)
  , xpNext(5)
{
  while (level + prestige < initialLevel)
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

void Experience::gain(unsigned xpGained, unsigned xpBonus, bool xpBoost)
{
  if (xpBoost)
    xpGained += xpGained / 2;

  xpGained += xpBonus;

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
  {
    ++level;
    ++unmodifiedLevel;
  }
  else
    ++prestige;
  xp = 0;
  if (veteran)
  {
    // Increments alternate between +4 and +5
    xpStep = 9 - xpStep;
  }
  xpNext += xpStep;
}

void Experience::modifyLevelBy(int delta)
{
  level = static_cast<unsigned>(std::min(std::max(static_cast<int>(level) + delta, 1), 10));
}

unsigned Experience::getUnmodifiedLevel() const
{
  return unmodifiedLevel;
}

unsigned Experience::forHeroAndMonsterLevels(unsigned heroLevel, unsigned monsterLevel)
{
  if (monsterLevel > heroLevel)
  {
    const auto delta = monsterLevel - heroLevel;
    return monsterLevel + delta * (delta - 1) + 2;
  }
  return monsterLevel;
}
