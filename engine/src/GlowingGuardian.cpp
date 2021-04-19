#include "engine/GlowingGuardian.hpp"

#include <algorithm>

GlowingGuardian::GlowingGuardian()
  : generator(std::random_device{}())
{
}

bool GlowingGuardian::canUseAbsolution(int heroLevel, const Monsters& monsters) const
{
  return std::find_if(begin(monsters), end(monsters), [maxLevel = std::min(heroLevel, 9)](auto& monster) {
           return monster.getLevel() <= maxLevel && !monster.isUndead();
         }) != end(monsters);
}

Monsters::iterator GlowingGuardian::pickMonsterForAbsolution(int heroLevel, Monsters& monsters)
{
  std::vector<Monsters::iterator> lowerLevelMonsters;
  const int maxLevel = std::min(heroLevel, 9);
  for (auto monsterIt = begin(monsters); monsterIt != end(monsters); ++monsterIt)
  {
    if (monsterIt->getLevel() <= maxLevel && !monsterIt->isUndead())
      lowerLevelMonsters.emplace_back(monsterIt);
  }
  if (lowerLevelMonsters.empty())
    return end(monsters);
  return lowerLevelMonsters[std::uniform_int_distribution<>(0, lowerLevelMonsters.size() - 1)(generator)];
}
