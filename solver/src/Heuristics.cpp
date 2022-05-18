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

  OneShotResult checkOneShot(const Hero& hero, const Monster& monster)
  {
    const auto monsterDamageTaken = monster.predictDamageTaken(hero.getDamageOutputVersus(monster), hero.damageType());
    const bool willDefeatMonster = monsterDamageTaken >= monster.getHitPoints();
    if (willDefeatMonster)
    {
      if (hero.hasInitiativeVersus(monster))
        return OneShotResult::VictoryFlawless;
      if (isSafeToAttack(hero, monster))
        return OneShotResult::VictoryDamaged;
      if (hero.has(HeroStatus::DeathProtection))
        return OneShotResult::VictoryDeathProtectionLost;
      const bool firstStrikeHero =
          hero.has(HeroStatus::FirstStrikePermanent) || hero.has(HeroStatus::FirstStrikeTemporary);
      const bool canCast = !firstStrikeHero && hero.has(Spell::Getindare) &&
                           hero.getManaPoints() >= Magic::spellCosts(Spell::Getindare, hero);
      const bool firstStrikeMonster = monster.has(MonsterTrait::FirstStrike) && !monster.isSlowed();
      if (canCast && !firstStrikeMonster)
        return OneShotResult::VictoryGetindareOnly;
    }
    if (!isSafeToAttack(hero, monster))
      return OneShotResult::Danger;
    return OneShotResult::None;
  }

  CatapultResult checkLevelCatapult(const Hero& hero, const Monsters& monsters)
  {
    auto totalXp = 0_xp;
    auto xpTakingDamage = 0_xp;
    auto xpBarelyWin = 0_xp;
    for (const auto& monster : monsters)
    {
      const auto oneShot = checkOneShot(hero, monster);
      if (oneShot == OneShotResult::None || oneShot == OneShotResult::Danger)
        continue;
      const auto xpGain = ExperiencePoints{hero.predictExperienceForKill(monster.getLevel(), monster.isSlowed())};
      if (oneShot == OneShotResult::VictoryFlawless)
        totalXp += xpGain;
      else if (oneShot == OneShotResult::VictoryDamaged)
        xpTakingDamage = std::max(xpTakingDamage, xpGain);
      else if (oneShot == OneShotResult::VictoryDeathProtectionLost || oneShot == OneShotResult::VictoryGetindareOnly)
        xpBarelyWin = std::max(xpBarelyWin, xpGain);
    }
    const auto xpNeeded = ExperiencePoints{hero.getXPforNextLevel() - hero.getXP()};
    if (totalXp >= xpNeeded)
      return CatapultResult::Flawless;
    if (totalXp + xpTakingDamage >= xpNeeded)
      return CatapultResult::Damaged;
    if (totalXp + xpTakingDamage + xpBarelyWin >= xpNeeded && hero.has(HeroStatus::DeathProtection))
    {
      if (totalXp + xpBarelyWin >= xpNeeded)
        return CatapultResult::DeathProtectionLost;
      return CatapultResult::DamagedAndDeathProtectionLost;
    }
    return CatapultResult::None;
  }

  namespace
  {
    thread_local Monsters ignoreMonsters;
    thread_local SimpleResources ignoreResources;
  } // namespace

  bool checkMeleeOnly(Hero hero, Monster monster)
  {
    // Deny dodging
    hero.add(HeroStatus::Pessimist);

    while (!hero.isDefeated())
    {
      Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
      if (monster.isDefeated())
        return true;
    }
    return false;
  }

  inline bool canRecover(const Hero& hero)
  {
    return hero.getHitPoints() < hero.getHitPointsMax() && !hero.has(HeroDebuff::Poisoned) &&
           !hero.has(HeroStatus::Manaform) && (!hero.has(HeroTrait::Herbivore) || hero.getFoodCount() > 0);
  }

  inline void doRecovery(Hero& hero, Monster& monster, RegenFightResult& result)
  {
    hero.recover(1u, ignoreMonsters);
    monster.recover(1u);
    ++result.numSquares;
  };

  std::optional<RegenFightResult> checkRegenFight(Hero hero, Monster monster)
  {
    RegenFightResult result;

    if (hero.isDefeated())
      return {};
    if (monster.isDefeated())
      return {std::move(result)};

    // Deny dodging
    hero.add(HeroStatus::Pessimist);

    // If recovery is needed to win the fight, it is always(?) better to do it first
    while (canRecover(hero) && !checkMeleeOnly(hero, monster))
      doRecovery(hero, monster, result);

    std::optional<uint16_t> lowestMonsterHpOnAttack;

    do
    {
      switch (checkOneShot(hero, monster))
      {
      case OneShotResult::None:
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
      case OneShotResult::VictoryFlawless:
      case OneShotResult::VictoryDamaged:
      case OneShotResult::VictoryDeathProtectionLost:
        ++result.numAttacks;
        return {std::move(result)};
      case OneShotResult::VictoryGetindareOnly:
      case OneShotResult::Danger:
        if (!canRecover(hero))
          return {};
        doRecovery(hero, monster, result);
      }
    } while (result.numSquares < 400u);
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
        result.numSquares += recover();
        if (result.numSquares > 400u)
          return {};
      }
      heroHitPoints -= heroHpLossPerAttack;
      monsterTotalHpLoss += monsterHpLossPerAttack;
    } while (monsterTotalHpLoss < monster.getHitPoints());
    return {std::move(result)};
  }

  std::optional<CatapultRegenFightResult> checkRegenFightWithCatapult(Hero hero, Monster monster)
  {
/*    auto printstats = [&] {
      std::cout << hero.getHitPoints() << "/" << hero.getHitPointsMax() << ":" << monster.getHitPoints() << "/"
                << monster.getHitPointsMax() << ", ";
    };
    */
    RegenFightResult before;
    while (isSafeToAttack(hero, monster))
    {
      [[maybe_unused]] const auto summary = Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
      assert(summary != Summary::Death && summary != Summary::Petrified && summary != Summary::NotPossible);
      before.numAttacks++;
      if (monster.isDefeated())
        return {{.beforeCatapult = std::move(before)}};
    }

    // It may be worthwhile to recover just enough to get one more hit in before levelling
    const auto optionA = [&, hero, monster, before]() mutable -> std::optional<CatapultRegenFightResult> {
      const auto monsterHpBeforeRecovery = monster.getHitPoints();
      while (!isSafeToAttack(hero, monster))
        doRecovery(hero, monster, before);
      [[maybe_unused]] const auto summary = Combat::attack(hero, monster, ignoreMonsters, ignoreResources);
      assert(summary != Summary::Death && summary != Summary::Petrified && summary != Summary::NotPossible);
      if (monster.getHitPoints() >= monsterHpBeforeRecovery)
        return {};
      before.numAttacks++;
      if (monster.isDefeated())
        return {{.beforeCatapult = std::move(before)}};
      hero.gainLevel(ignoreMonsters);
      if (const auto after = checkRegenFight(std::move(hero), std::move(monster)))
        return {{.beforeCatapult = std::move(before), .afterCatapult = std::move(*after)}};
      else
        return {};
    }();

    const auto optionB = [&, hero, monster]() mutable -> std::optional<CatapultRegenFightResult> {
      hero.gainLevel(ignoreMonsters);
      if (const auto after = checkRegenFight(std::move(hero), std::move(monster)))
        return {{.beforeCatapult = std::move(before), .afterCatapult = std::move(*after)}};
      else
        return {};
    }();

    if (optionA && (!optionB || optionA->numSquares() < optionB->numSquares()))
      return optionA;
    return optionB;
  }
} // namespace heuristics

std::optional<Solution> runHeuristics(GameState)
{
  return {{Attack{}}};
}
