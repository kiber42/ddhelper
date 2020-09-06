#include "Experience.hpp"

#include <algorithm>
#include <cassert>

Experience::Experience(bool hasVeteranTrait)
  : level(1)
  , prestige(0)
  , veteran(hasVeteranTrait)
  , xp(0)
  , xpStep(hasVeteranTrait ? 4 : 5)
  , xpNext(xpStep)
{
}

void Experience::gain(int xpGained, int xpBonus, bool xpBoost)
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
    ++level;
  else
    ++prestige;
  xp = 0;
  if (veteran)
  {
    // Alternate between +4 and +5
    xpStep = 9 - xpStep;
  }
  xpNext += xpStep;
}

void Experience::modifyLevelBy(int delta)
{
  level = std::min(std::max(level + delta, 1), 10);
}

int Experience::forHeroAndMonsterLevels(int heroLevel, int monsterLevel)
{
  const int delta = monsterLevel - heroLevel;
  return monsterLevel + (delta > 0 ? delta * (delta - 1) + 2 : 0);
}
