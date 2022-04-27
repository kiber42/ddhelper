#include "solver/Heuristics.hpp"

#include "engine/Magic.hpp"

namespace heuristics
{
  const Monster& strongest(const Monsters& monsters)
  {
    assert(!monsters.empty());
    auto indices = std::vector<size_t>(monsters.size());
    std::iota(begin(indices), end(indices), 0);
    std::sort(begin(indices), end(indices), [&monsters](auto indexA, auto indexB) {
      return monsters[indexA].getLevel() > monsters[indexB].getLevel();
    });
    return monsters[indices.front()];
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
