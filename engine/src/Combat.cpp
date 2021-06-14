#include "engine/Combat.hpp"

#include "engine/Items.hpp"

#include <cassert>
#include <iostream>

namespace Combat
{
  namespace
  {
    // Determines outcome summary and awards experience if applicable.
    Summary summaryAndExperience(
        Hero& hero, const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning, Monsters& allMonsters)
    {
      assert(!hero.isDefeated());

      if (!monster.isDefeated())
        return Summary::Safe;

      const int levelBefore = hero.getLevel() + hero.getPrestige();
      hero.monsterKilled(monster, monsterWasSlowed, monsterWasBurning, allMonsters);
      if (hero.getLevel() + hero.getPrestige() > levelBefore)
        return Summary::LevelUp;
      return Summary::Win;
    }

    // Evaluate effect of burn down after another monster has been attacked
    void attackedOther(Hero& hero, Monster& monster, Monsters& allMonsters)
    {
      if (!monster.isBurning())
        return;
      const bool monsterWasSlowed = monster.isSlowed();
      monster.burnDown();
      if (monster.isDefeated())
        hero.monsterKilled(monster, monsterWasSlowed, true, allMonsters);
    }

    void applyLifeSteal(Hero& hero, const Monster& monster, int monsterHitPointsBefore)
    {
      if (hero.hasStatus(HeroStatus::LifeSteal) && !monster.has(MonsterTrait::Bloodless))
      {
        const int multiplier = monster.getLevel() < hero.getLevel() ? 2 : 1;
        const int damageDealt = monsterHitPointsBefore - monster.getHitPoints();
        const int healthStolen =
            std::min(hero.getStatusIntensity(HeroStatus::LifeSteal) * hero.getLevel() * multiplier, damageDealt);
        if (healthStolen > 0)
        {
          hero.healHitPoints(healthStolen, true);
          hero.collect(hero.getFaith().lifeStolen(monster));
        }
      }
    }

    Summary knockBackMonster(Hero& hero, Monster& monster, Monsters& allMonsters, Monster* intoMonster)
    {
      const int knockback = hero.getStatusIntensity(HeroStatus::Knockback);
      if (knockback == 0)
        return Summary::Safe;
      const bool monsterWasSlowed = monster.isSlowed();
      const bool monsterWasBurning = monster.isBurning();
      if (intoMonster == nullptr)
        monster.takeDamage(hero.getBaseDamage() * knockback, DamageType::Physical);
      else
      {
        const int damageOutput = hero.getBaseDamage() * knockback * 8 / 10;
        const int effectiveDamage = monster.predictDamageTaken(damageOutput, DamageType::Physical);
        monster.takeDamage(effectiveDamage, DamageType::Typeless);
        const int maxSecondaryDamage = intoMonster->getHitPoints() - 1;
        if (maxSecondaryDamage > 0)
          intoMonster->takeDamage(std::min(effectiveDamage, maxSecondaryDamage), DamageType::Typeless);
      }
      return summaryAndExperience(hero, monster, monsterWasSlowed, monsterWasBurning, allMonsters);
    }
  } // namespace

  namespace detail
  {
    Summary finalizeAttack(Hero& hero,
                           const Monster& monster,
                           bool monsterWasSlowed,
                           bool monsterWasBurning,
                           bool triggerBurndown,
                           Monsters& allMonsters)
    {
      if (hero.isDefeated())
        return Summary::Death;

      auto summary = summaryAndExperience(hero, monster, monsterWasSlowed, monsterWasBurning, allMonsters);

      if (triggerBurndown)
      {
        const int levelBefore = hero.getLevel() + hero.getPrestige();
        for (auto& otherMonster : allMonsters)
        {
          if (otherMonster != monster)
            attackedOther(hero, otherMonster, allMonsters);
        }
        if (hero.getLevel() + hero.getPrestige() > levelBefore)
          summary = Summary::LevelUp;
      }

      hero.applyCollectedPiety(allMonsters);

      return summary;
    }
  } // namespace detail

  // Perform melee attack on monster, evaluate effects on all monsters
  Summary attack(Hero& hero, Monster& monster, Monsters& allMonsters)
  {
    if (hero.isDefeated())
    {
      std::cerr << "Dead hero cannot fight." << std::endl;
      return Summary::NotPossible;
    }
    if (monster.isDefeated())
    {
      std::cerr << "Cannot fight defeated monster." << std::endl;
      return Summary::NotPossible;
    }

    // Bonus experience is added if the monster was slowed before the final attack
    bool monsterWasSlowed = monster.isSlowed();
    bool monsterWasBurning = monster.isBurning();
    bool heroReceivedHit = false;
    bool appliedPoison = false;

    const int monsterDamageInitial = monster.getDamage();
    const bool reflexes = hero.hasStatus(HeroStatus::Reflexes);
    const bool swiftHand = hero.hasTrait(HeroTrait::SwiftHand) && hero.getLevel() > monster.getLevel();
    const bool willPetrify = !hero.hasStatus(HeroStatus::DeathGazeImmune) &&
                             (monster.getDeathGazePercent() * hero.getHitPointsMax() > hero.getHitPoints() * 100);

    auto heroAttacks = [&] {
      const int monsterHPBefore = monster.getHitPoints();
      if (hero.hasStatus(HeroStatus::CrushingBlow))
        monster.receiveCrushingBlow();
      else if (hero.hasStatus(HeroStatus::BurningStrike))
      {
        monster.takeBurningStrikeDamage(hero.getDamageOutputVersus(monster), hero.getLevel(), hero.damageType());
        if (hero.hasStatus(HeroStatus::HeavyFireball))
          monster.burnMax(2 * hero.getLevel());
      }
      else
        monster.takeDamage(hero.getDamageOutputVersus(monster), hero.damageType());
      applyLifeSteal(hero, monster, monsterHPBefore);
      if (!monster.isDefeated() && hero.hasStatus(HeroStatus::Poisonous))
      {
        const int poisonAmount = hero.getStatusIntensity(HeroStatus::Poisonous) * hero.getLevel();
        if (monster.poison(poisonAmount))
          appliedPoison = true;
      }
      if (hero.hasStatus(HeroStatus::CorrosiveStrike))
        monster.corrode(hero.getStatusIntensity(HeroStatus::CorrosiveStrike));
      if (hero.hasStatus(HeroStatus::Might))
        monster.erodeResitances();
      hero.removeOneTimeAttackEffects();
    };

    auto monsterAttacks = [&] {
      if (!hero.tryDodge(allMonsters))
      {
        if (monster.has(MonsterTrait::CurseBearer))
          hero.addStatus(HeroDebuff::Cursed, allMonsters);
        if (willPetrify)
        {
          // Hero either dies or death protection is triggered
          hero.loseHitPointsOutsideOfFight(hero.getHitPoints(), allMonsters);
          heroReceivedHit = true;
        }
        else
        {
          heroReceivedHit = hero.takeDamage(monsterDamageInitial, monster.damageType(), allMonsters);
        }
        if (hero.hasTrait(HeroTrait::ManaShield) && heroReceivedHit && !hero.isDefeated())
          monster.takeManaShieldDamage(hero.getLevel());
      }
    };

    hero.startPietyCollection();

    if (swiftHand)
    {
      monster.die();
    }
    else if (hero.hasInitiativeVersus(monster))
    {
      heroAttacks();
      if (!monster.isDefeated())
      {
        // If the monster is defeated beyond this point (Reflexes or Mana Shield),
        // it was not slowed nor burning (except if attacked with Burning Strike) before the final blow.
        monsterWasSlowed = false;
        monsterWasBurning = hero.hasStatus(HeroStatus::BurningStrike);
        monsterAttacks();
        if (reflexes && !hero.isDefeated() && !monster.isDefeated())
          heroAttacks();
      }
    }
    else
    {
      monsterAttacks();
      if (!hero.isDefeated())
        heroAttacks();
    }

    if (heroReceivedHit)
    {
      if (monster.has(MonsterTrait::Poisonous))
        hero.addStatus(HeroDebuff::Poisoned, allMonsters);
      if (monster.has(MonsterTrait::ManaBurn))
        hero.addStatus(HeroDebuff::ManaBurned, allMonsters);
      if (monster.has(MonsterTrait::Corrosive))
        hero.addStatus(HeroDebuff::Corroded, allMonsters);
      if (monster.has(MonsterTrait::Weakening))
        hero.addStatus(HeroDebuff::Weakened, allMonsters);
      hero.collect(hero.getFaith().receivedHit(monster));
    }

    if (appliedPoison)
    {
      // Applying poison twice (with reflexes) does not trigger likes/dislikes twice
      hero.collect(hero.getFaith().monsterPoisoned(monster));
    }

    hero.adjustMomentum(monster.isDefeated());

    if (willPetrify && hero.isDefeated())
      return Summary::Petrified;

    return detail::finalizeAttack(hero, monster, monsterWasSlowed, monsterWasBurning, true, allMonsters);
  }

  Summary
  attackWithKnockback(Hero& hero, Monster& primary, Monsters& allMonsters, Knockback knockback, Resources& resources)
  {
    const bool reflexes = hero.hasStatus(HeroStatus::Reflexes);
    auto summary = attack(hero, primary, allMonsters);
    if (summary == Summary::Safe && hero.hasStatus(HeroStatus::Knockback))
    {
      switch (knockback.targetType)
      {
      case Knockback::TargetType::Empty:
      case Knockback::TargetType::Indestructible:
        break;
      case Knockback::TargetType::Monster:
        summary = knockBackMonster(hero, primary, allMonsters, knockback.monster);
        if (reflexes && summary == Summary::Safe)
          summary = knockBackMonster(hero, primary, allMonsters, knockback.monster);
        break;
      case Knockback::TargetType::Wall:
        if (!primary.has(MonsterTrait::Cowardly))
        {
          auto& resourceSet = resources();
          assert(resourceSet.numWalls > 0);
          summary = knockBackMonster(hero, primary, allMonsters, nullptr);
          --resourceSet.numWalls;
          if (reflexes && summary == Summary::Safe && resourceSet.numWalls > 0)
          {
            summary = knockBackMonster(hero, primary, allMonsters, nullptr);
            --resourceSet.numWalls;
          }
        }
        break;
      }
    }
    return summary;
  }
} // namespace Combat
