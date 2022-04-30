#include "solver/Heuristics.hpp"

#include "engine/Magic.hpp"

namespace heuristics
{
  const Monster& strongest(const Monsters& monsters)
  {
    assert(!monsters.empty());
    return *std::max_element(begin(monsters), end(monsters),
                             [](auto& left, auto& right) { return left.getLevel() < right.getLevel(); });
  }

  std::vector<const Monster*> sorted_by_level(const Monsters& monsters)
  {
    std::vector<const Monster*> sorted(monsters.size());
    std::transform(begin(monsters), end(monsters), begin(sorted), [](const auto& monster) { return &monster; });
    std::sort(begin(sorted), end(sorted), [](auto left, auto right) { return left->getLevel() > right->getLevel(); });
    return sorted;
  }

  OneShotType checkOneShot(const Hero& hero, const Monster& monster)
  {
    const auto monsterDamageTaken = monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType());
    const bool willDefeatMonster = monsterDamageTaken >= monster.getHitPoints();
    if (willDefeatMonster)
    {
      if (hero.hasInitiativeVersus(monster))
        return OneShotType::Flawless;
      if (hero.predictDamageTaken(monster.getDamage(), monster.damageType()) < hero.getHitPoints())
        return OneShotType::Damaged;
      const bool firstStrikeHero =
          hero.has(HeroStatus::FirstStrikePermanent) || hero.has(HeroStatus::FirstStrikeTemporary);
      const bool canCast = !firstStrikeHero && hero.has(Spell::Getindare) &&
                           hero.getManaPoints() >= Magic::spellCosts(Spell::Getindare, hero);
      const bool firstStrikeMonster = monster.has(MonsterTrait::FirstStrike) && !monster.isSlowed();
      if (canCast && !firstStrikeMonster)
        return OneShotType::GetindareOnly;
    }
    return OneShotType::None;
  }

  bool checkLevelCatapult(const Hero& hero, const Monsters& monsters)
  {
    auto oneShotXp = 0_xp;
    auto oneShotFinalXp = 0_xp;
    for (const auto& monster : monsters)
    {
      const auto oneShot = checkOneShot(hero, monster);
      if (oneShot == OneShotType::Flawless)
        oneShotXp += ExperiencePoints{hero.predictExperienceForKill(monster.getLevel(), monster.isSlowed())};
      else if (oneShot == OneShotType::Damaged || oneShot == OneShotType::GetindareOnly)
        oneShotFinalXp = std::max(
            oneShotFinalXp, ExperiencePoints{hero.predictExperienceForKill(monster.getLevel(), monster.isSlowed())});
    }
    return hero.getXP() + (oneShotXp + oneShotFinalXp).get() >= hero.getXPforNextLevel();
  }
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState)
{
  return {{Attack{}}};
}
