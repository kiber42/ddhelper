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

  inline bool isSafeToAttack(const Hero& hero, const Monster& monster)
  {
    const bool willPetrify = !monster.isSlowed() && !hero.has(HeroStatus::DeathGazeImmune) &&
                             (monster.getDeathGazePercent() * hero.getHitPointsMax() > hero.getHitPoints() * 100);
    const bool deadly =
        (willPetrify || hero.predictDamageTaken(monster.getDamage(), monster.damageType()) >= hero.getHitPoints());
    return !deadly;
  }

  OneShotType checkOneShot(const Hero& hero, const Monster& monster)
  {
    const auto monsterDamageTaken = monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType());
    const bool willDefeatMonster = monsterDamageTaken >= monster.getHitPoints();
    if (willDefeatMonster)
    {
      if (hero.hasInitiativeVersus(monster))
        return OneShotType::VictoryFlawless;
      if (isSafeToAttack(hero, monster))
        return OneShotType::VictoryDamaged;
      if (hero.has(HeroStatus::DeathProtection))
        return OneShotType::VictoryDeathProtectionLost;
      const bool firstStrikeHero =
          hero.has(HeroStatus::FirstStrikePermanent) || hero.has(HeroStatus::FirstStrikeTemporary);
      const bool canCast = !firstStrikeHero && hero.has(Spell::Getindare) &&
                           hero.getManaPoints() >= Magic::spellCosts(Spell::Getindare, hero);
      const bool firstStrikeMonster = monster.has(MonsterTrait::FirstStrike) && !monster.isSlowed();
      if (canCast && !firstStrikeMonster)
        return OneShotType::VictoryGetindareOnly;
    }
    if (!isSafeToAttack(hero, monster))
      return OneShotType::Danger;
    return OneShotType::None;
  }

  bool checkLevelCatapult(const Hero& hero, const Monsters& monsters)
  {
    auto oneShotXp = 0_xp;
    auto oneShotFinalXp = 0_xp;
    for (const auto& monster : monsters)
    {
      const auto oneShot = checkOneShot(hero, monster);
      if (oneShot == OneShotType::VictoryFlawless)
        oneShotXp += ExperiencePoints{hero.predictExperienceForKill(monster.getLevel(), monster.isSlowed())};
      else if (oneShot == OneShotType::VictoryDamaged || oneShot == OneShotType::VictoryGetindareOnly)
        oneShotFinalXp = std::max(
            oneShotFinalXp, ExperiencePoints{hero.predictExperienceForKill(monster.getLevel(), monster.isSlowed())});
    }
    return hero.getXP() + (oneShotXp + oneShotFinalXp).get() >= hero.getXPforNextLevel();
  }

  namespace
  {
    thread_local Monsters ignoreMonsters;
    thread_local SimpleResources ignoreResources;
  } // namespace

  std::optional<RegenFightResult> checkRegenFight(Hero hero, Monster monster)
  {
    RegenFightResult result;

    if (hero.isDefeated())
      return {};
    if (monster.isDefeated())
      return {std::move(result)};

    auto recoveryAvailable = [&] {
      if (hero.getHitPoints() >= hero.getHitPointsMax())
        return false;
      monster.recover(1u);
      hero.recover(1u, ignoreMonsters);
      ++result.numSquaresUncovered;
      return true;
    };

    std::optional<uint16_t> lowestMonsterHpOnAttack;

    do
    {
      switch (checkOneShot(hero, monster))
      {
      case OneShotType::None:
      {
        // confirm that we're making progress
        const auto monsterHitPoints = monster.getHitPoints();
        if (lowestMonsterHpOnAttack && monsterHitPoints >= *lowestMonsterHpOnAttack)
          return {};
        lowestMonsterHpOnAttack = monsterHitPoints;
        [[maybe_unused]] const auto summary = Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
        assert(summary != Summary::Death && summary != Summary::Petrified && summary != Summary::NotPossible);
        ++result.numAttacks;
        if (monster.isDefeated())
          return {std::move(result)};
        break;
      }
      case OneShotType::VictoryFlawless:
      case OneShotType::VictoryDamaged:
      case OneShotType::VictoryDeathProtectionLost:
        ++result.numAttacks;
        return {std::move(result)};
      case OneShotType::VictoryGetindareOnly:
      case OneShotType::Danger:
        if (!recoveryAvailable())
          return {};
      }
    } while (result.numSquaresUncovered < 400u);
    return {};
  }

  std::optional<RegenFightResult> checkRegenFightFast(const Hero& hero, const Monster& monster)
  {
    const auto heroHpLossPerAttack = hero.predictDamageTaken(monster.getDamage(), monster.damageType());
    const auto burnStackSize = monster.getBurnStackSize();
    const auto monsterHpLossPerAttack =
        monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType()) - burnStackSize;
    const bool heroInitiative = hero.hasInitiativeVersus(monster);
    const bool justOneHit = monsterHpLossPerAttack + burnStackSize >= monster.getHitPoints();
    if (heroInitiative && justOneHit)
      return {{.numAttacks = 1}};
    if (monsterHpLossPerAttack == 0u)
      return {};
    if (heroHpLossPerAttack == 0u)
      return {{.numAttacks = static_cast<unsigned>(ceil(1.0 * monster.getHitPoints() / monsterHpLossPerAttack))}};

    // Evaluate initial combat, before any recovery is needed
    const auto numAttacksBeforeRecovery = (hero.getHitPoints() - 1) / heroHpLossPerAttack;
    auto heroHitPoints = hero.getHitPoints() - heroHpLossPerAttack * numAttacksBeforeRecovery;
    auto monsterTotalHpLoss = monsterHpLossPerAttack * numAttacksBeforeRecovery + burnStackSize;
    if (heroInitiative && hero.hasInitiativeVersusIgnoreMonsterSlowed(monster))
      monsterTotalHpLoss += monsterHpLossPerAttack;
    if (monsterTotalHpLoss >= monster.getHitPoints())
      return {{.numAttacks = numAttacksBeforeRecovery}};
    if (hero.has(HeroDebuff::Poisoned) || hero.has(HeroStatus::Manaform))
      return {};

    // Setup regen fighting evaluation
    const auto heroRecoveryRate = hero.recoveryMultiplier();
    const auto monsterRecoveryRate = monster.getLevel() * (monster.has(MonsterTrait::FastRegen) ? 2u : 1u);
    auto monsterPoison = monster.getPoisonAmount();
    if (monsterPoison == 0 && monsterHpLossPerAttack * heroRecoveryRate < heroHpLossPerAttack * monsterRecoveryRate)
      return {};

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

    RegenFightResult result{.numAttacks = numAttacksBeforeRecovery};

    // Simulate regen fighting
    do
    {
      if (heroHitPoints <= heroHpLossPerAttack)
      {
        result.numSquaresUncovered += recover();
        if (result.numSquaresUncovered > 400u)
          return {};
      }
      heroHitPoints -= heroHpLossPerAttack;
      monsterTotalHpLoss += monsterHpLossPerAttack;
    } while (monsterTotalHpLoss < monster.getHitPoints());
    return {std::move(result)};
  }
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState)
{
  return {{Attack{}}};
}