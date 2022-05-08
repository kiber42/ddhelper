#include "solver/Heuristics.hpp"

#include "engine/Combat.hpp"
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
      if (hero.has(HeroStatus::DeathProtection))
        return OneShotType::DeathProtectionLost;
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

  std::optional<unsigned> checkRegenFight(Hero hero, Monster monster)
  {
    thread_local Monsters ignoreMonsters;
    thread_local SimpleResources ignoreResources;
    if (hero.isDefeated())
      return {};
    if (monster.isDefeated())
      return 0;
    auto numSquaresUncovered = 0u;
    const auto monsterMaxHitPoints = monster.getHitPointsMax();
    do
    {
      while (true)
      {
        const bool willPetrify = !monster.isSlowed() && !hero.has(HeroStatus::DeathGazeImmune) &&
                                 (monster.getDeathGazePercent() * hero.getHitPointsMax() > hero.getHitPoints() * 100);
        const bool deadly =
            (willPetrify || hero.predictDamageTaken(monster.getDamage(), monster.damageType()) >= hero.getHitPoints());
        if (!deadly)
        {
          [[maybe_unused]] const auto summary = Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
          assert(summary != Summary::Death && summary != Summary::Petrified && summary != Summary::NotPossible);
          if (monster.isDefeated())
            return numSquaresUncovered;
        }
        else
        {
          const auto oneShotResult = checkOneShot(hero, monster);
          if (oneShotResult == OneShotType::Flawless || oneShotResult == OneShotType::DeathProtectionLost)
            return numSquaresUncovered;
          if (oneShotResult == OneShotType::Damaged && !willPetrify)
            return numSquaresUncovered;
          if (hero.getHitPoints() >= hero.getHitPointsMax())
            return {};
          break;
        }
      }

      if (monster.getHitPoints() < monsterMaxHitPoints)
      {
        monster.recover(1u);
        if (monster.getHitPoints() == monsterMaxHitPoints)
          return {};
      }
      hero.recover(1u, ignoreMonsters);
      ++numSquaresUncovered;
    } while (numSquaresUncovered < 400u);
    return {};
  }

  std::optional<unsigned> checkRegenFightFast(const Hero& hero, const Monster& monster)
  {
    const auto heroHpLossPerAttack = hero.predictDamageTaken(monster.getDamage(), monster.damageType());
    const auto burnStackSize = monster.getBurnStackSize();
    const auto monsterHpLossPerAttack =
        monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType()) - burnStackSize;
    const bool heroInitiative = hero.hasInitiativeVersus(monster);
    const bool justOneHit = monsterHpLossPerAttack + burnStackSize >= monster.getHitPoints();
    if (heroInitiative && justOneHit)
      return 0;
    if (monsterHpLossPerAttack == 0u)
      return {};
    if (heroHpLossPerAttack == 0u)
      return 0;

    // Evaluate initial combat, before any recovery is needed
    const auto numAttacksBeforeRecovery = (hero.getHitPoints() - 1) / heroHpLossPerAttack;
    auto heroHitPoints = hero.getHitPoints() - heroHpLossPerAttack * numAttacksBeforeRecovery;
    auto monsterTotalHpLoss = monsterHpLossPerAttack * numAttacksBeforeRecovery + burnStackSize;
    if (heroInitiative && hero.hasInitiativeVersusIgnoreMonsterSlowed(monster))
      monsterTotalHpLoss += monsterHpLossPerAttack;
    if (monsterTotalHpLoss >= monster.getHitPoints())
      return 0;
    if (hero.has(HeroDebuff::Poisoned) || hero.has(HeroStatus::Manaform))
      return {};

    // Setup regen fighting evaluation
    const auto heroRecoveryRate = hero.recoveryMultiplier();
    const auto monsterRecoveryRate = monster.getLevel() * (monster.has(MonsterTrait::FastRegen) ? 2u : 1u);
    auto monsterPoison = monster.getPoisonAmount();
    if (monsterPoison == 0 && monsterHpLossPerAttack * heroRecoveryRate < heroHpLossPerAttack * monsterRecoveryRate)
      return {};
    auto squaresUncovered = 0u;
    auto recover = [&] {
      auto uncoverForNextAttack =
          static_cast<unsigned>(ceil(1.0 * (heroHpLossPerAttack + 1 - heroHitPoints) / heroRecoveryRate));
      heroHitPoints += uncoverForNextAttack * heroRecoveryRate;
      if (monsterPoison < uncoverForNextAttack)
      {
        if (monsterPoison > 0)
        {
          uncoverForNextAttack -= monsterPoison;
          monsterPoison = 0;
        }
        const auto recoveredHitPoints = uncoverForNextAttack * monsterRecoveryRate;
        if (recoveredHitPoints >= monsterTotalHpLoss)
        {
          // Not winnable; can only get here for poisoned monster
          monsterTotalHpLoss = 0;
          return 1000u;
        }
        monsterTotalHpLoss -= uncoverForNextAttack * monsterRecoveryRate;
      }
      else
      {
        monsterPoison -= uncoverForNextAttack;
      }
      return uncoverForNextAttack;
    };

    // Simulate regen fighting
    do
    {
      if (heroHitPoints <= heroHpLossPerAttack)
      {
        squaresUncovered += recover();
        if (squaresUncovered > 400u)
          return {};
      }
      heroHitPoints -= heroHpLossPerAttack;
      monsterTotalHpLoss += monsterHpLossPerAttack;
    } while (monsterTotalHpLoss < monster.getHitPoints());
    return squaresUncovered;
  }
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState)
{
  return {{Attack{}}};
}
