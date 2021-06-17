#include "engine/GlowingGuardian.hpp"

#include <algorithm>

GlowingGuardian::GlowingGuardian()
  : generator(std::random_device{}())
{
}

bool GlowingGuardian::canUseAbsolution(unsigned heroLevel, const Monsters& monsters) const
{
  return std::find_if(begin(monsters), end(monsters), [maxLevel = std::min(heroLevel, 9u)](auto& monster) {
           return monster.getLevel() <= maxLevel && !monster.has(MonsterTrait::Undead);
         }) != end(monsters);
}

Monsters::iterator GlowingGuardian::pickMonsterForAbsolution(unsigned heroLevel, Monsters& monsters)
{
  std::vector<Monsters::iterator> lowerLevelMonsters;
  const auto maxLevel = std::min(heroLevel, 9u);
  for (auto monsterIt = begin(monsters); monsterIt != end(monsters); ++monsterIt)
  {
    if (monsterIt->getLevel() <= maxLevel && !monsterIt->has(MonsterTrait::Undead))
      lowerLevelMonsters.emplace_back(monsterIt);
  }
  if (lowerLevelMonsters.empty())
    return end(monsters);
  return lowerLevelMonsters[std::uniform_int_distribution<size_t>(0, lowerLevelMonsters.size() - 1)(generator)];
}
