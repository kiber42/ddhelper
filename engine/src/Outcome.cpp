#include "Outcome.hpp"

#include "Hero.hpp"

std::string toString(const Outcome& outcome)
{
  if (outcome.summary == Summary::Death)
    return toString(Summary::Death);

  std::string result;
  // At most one debuff is shown in string
  if (outcome.debuffs.count(Debuff::LostDeathProtection))
  {
    if (outcome.summary == Summary::LevelUp)
      result = "Barely Level";
    else if (outcome.summary == Summary::Win)
      result = "Barely Win";
    else if (outcome.summary == Summary::Safe)
      result = "Barely Alive";
  }
  else
  {
    result = toString(outcome.summary);
    if (outcome.debuffs.count(Debuff::Cursed))
      result += " Cursed";
    else if (outcome.debuffs.count(Debuff::ManaBurned))
      result += " Mana Burn";
    else if (outcome.debuffs.count(Debuff::Poisoned))
      result += " Poison";
    else if (outcome.debuffs.count(Debuff::Corroded))
      result += " Corroded";
    else if (outcome.debuffs.count(Debuff::Weakened))
      result += " Weaken";
    if (!result.empty() && result[0] == ' ')
      result.erase(0, 1);
  }
  if (outcome.pietyChange != 0)
  {
    if (!result.empty())
      result += " ";
    if (outcome.pietyChange > 0)
      result += "+" + std::to_string(outcome.pietyChange) + " piety";
    else
      result += std::to_string(outcome.pietyChange) + " piety";
  }
  return result;
}

Debuffs findDebuffs(const Hero& before, const Hero& after)
{
  Debuffs debuffs;
  if (before.hasStatus(HeroStatus::DeathProtection) && !after.hasStatus(HeroStatus::DeathProtection))
    debuffs.insert(Debuff::LostDeathProtection);
  if (before.getStatusIntensity(HeroStatus::Cursed) < after.getStatusIntensity(HeroStatus::Cursed))
    debuffs.insert(Debuff::Cursed);
  if (!before.hasStatus(HeroStatus::ManaBurned) && after.hasStatus(HeroStatus::ManaBurned))
    debuffs.insert(Debuff::ManaBurned);
  if (!before.hasStatus(HeroStatus::Poisoned) && after.hasStatus(HeroStatus::Poisoned))
    debuffs.insert(Debuff::Poisoned);
  if (before.getStatusIntensity(HeroStatus::Corrosion) < after.getStatusIntensity(HeroStatus::Corrosion))
    debuffs.insert(Debuff::Corroded);
  if (before.getStatusIntensity(HeroStatus::Weakened) < after.getStatusIntensity(HeroStatus::Weakened))
    debuffs.insert(Debuff::Weakened);
  return debuffs;
}
