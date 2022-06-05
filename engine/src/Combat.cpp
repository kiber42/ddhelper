#include "engine/Combat.hpp"

#include "engine/Items.hpp"

#include <cassert>
#include <iostream>

namespace Combat
{
  namespace
  {
    // Determines outcome summary and awards experience if applicable.
    Summary summaryAndExperience(Hero& hero,
                                 const Monster& monster,
                                 bool monsterWasSlowed,
                                 bool monsterWasBurning,
                                 Monsters& allMonsters,
                                 Resources& resources)
    {
      assert(!hero.isDefeated());

      if (!monster.isDefeated())
        return Summary::Safe;

      const auto levelBefore = hero.getLevel() + hero.getPrestige();
      hero.monsterKilled(monster, monsterWasSlowed, monsterWasBurning, allMonsters, resources);
      if (hero.getLevel() + hero.getPrestige() > levelBefore)
        return Summary::LevelUp;
      return Summary::Win;
    }

    // Evaluate effect of burn down after another monster has been attacked
    void attackedOther(Hero& hero, Monster& monster, Monsters& allMonsters, Resources& resources)
    {
      if (!monster.isBurning())
        return;
      const bool monsterWasSlowed = monster.isSlowed();
      monster.burnDown();
      if (monster.isDefeated())
        hero.monsterKilled(monster, monsterWasSlowed, true, allMonsters, resources);
    }

    void applyLifeSteal(Hero& hero, const Monster& monster, unsigned monsterHitPointsBefore)
    {
      if (hero.has(HeroStatus::LifeSteal) && !monster.has(MonsterTrait::Bloodless))
      {
        const auto multiplier = monster.getLevel() < hero.getLevel() ? 2u : 1u;
        const auto damageDealt = monsterHitPointsBefore - monster.getHitPoints();
        const auto healthStolen =
            std::min(hero.getIntensity(HeroStatus::LifeSteal) * hero.getLevel() * multiplier, damageDealt);
        if (healthStolen > 0)
        {
          hero.healHitPoints(healthStolen, true);
          hero.collect(hero.getFaith().lifeStolen(monster));
        }
      }
    }

    Summary
    knockBackMonster(Hero& hero, Monster& monster, Monsters& allMonsters, Monster* intoMonster, Resources& resources)
    {
      const auto knockback = hero.getIntensity(HeroStatus::Knockback);
      if (knockback == 0)
        return Summary::Safe;
      const bool monsterWasSlowed = monster.isSlowed();
      const bool monsterWasBurning = monster.isBurning();
      if (intoMonster == nullptr)
        monster.takeDamage(hero.getBaseDamage() * knockback, DamageType::Physical);
      else
      {
        const auto damageOutput = hero.getBaseDamage() * knockback * 8u / 10u;
        const auto effectiveDamage = monster.predictDamageTaken(damageOutput, DamageType::Physical);
        monster.takeDamage(effectiveDamage, DamageType::Typeless);
        const auto maxSecondaryDamage = intoMonster->getHitPoints() - 1u;
        if (maxSecondaryDamage > 0)
          intoMonster->takeDamage(std::min(effectiveDamage, maxSecondaryDamage), DamageType::Typeless);
      }
      return summaryAndExperience(hero, monster, monsterWasSlowed, monsterWasBurning, allMonsters, resources);
    }
  } // namespace

  namespace detail
  {
    Summary finalizeAttack(Hero& hero,
                           const Monster& monster,
                           bool monsterWasSlowed,
                           bool monsterWasBurning,
                           bool triggerBurndown,
                           Monsters& allMonsters,
                           Resources& resources)
    {
      if (hero.isDefeated())
        return Summary::Death;

      auto summary = summaryAndExperience(hero, monster, monsterWasSlowed, monsterWasBurning, allMonsters, resources);

      if (triggerBurndown)
      {
        const auto levelBefore = hero.getLevel() + hero.getPrestige();
        for (auto& otherMonster : allMonsters)
        {
          if (otherMonster != monster)
            attackedOther(hero, otherMonster, allMonsters, resources);
        }
        if (hero.getLevel() + hero.getPrestige() > levelBefore)
          summary = Summary::LevelUp;
      }

      hero.applyCollectedPiety(allMonsters);

      return summary;
    }

    void applyHitSideEffects(Hero& hero, const Monster& monster)
    {
      Monsters ignore;
      if (monster.has(MonsterTrait::Poisonous))
        hero.add(HeroDebuff::Poisoned, ignore);
      if (monster.has(MonsterTrait::ManaBurn))
        hero.add(HeroDebuff::ManaBurned, ignore);
      if (monster.has(MonsterTrait::Corrosive))
        hero.add(HeroDebuff::Corroded, ignore);
      if (monster.has(MonsterTrait::Weakening))
        hero.add(HeroDebuff::Weakened, ignore);
      hero.collect(hero.getFaith().receivedHit(monster));
    }
  } // namespace detail

  // Perform melee attack on monster, evaluate effects on all monsters
  Summary attack(Hero& hero, Monster& monster, Monsters& allMonsters, Resources& resources)
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

    const auto monsterDamageInitial = monster.getDamage();
    const bool reflexes = hero.has(HeroStatus::Reflexes);
    const bool swiftHand = hero.has(HeroTrait::SwiftHand) && hero.getLevel() > monster.getLevel();
    const bool heroUsesDeathGaze =
        hero.getIntensity(HeroStatus::DeathGaze) * monster.getHitPointsMax() > monster.getHitPoints() * 100;
    const bool willPetrify = !monster.isSlowed() && !hero.has(HeroStatus::DeathGazeImmune) &&
                             (monster.getDeathGazePercent() * hero.getHitPointsMax() > hero.getHitPoints() * 100u);

    auto heroAttacks = [&] {
      const auto monsterHPBefore = monster.getHitPoints();
      if (hero.has(HeroStatus::CrushingBlow))
        monster.receiveCrushingBlow();
      else if (hero.has(HeroStatus::BurningStrike))
      {
        monster.takeBurningStrikeDamage(hero.getDamageOutputVersus(monster), hero.getLevel(), hero.damageType());
        if (hero.has(HeroStatus::HeavyFireball))
          monster.burnMax(static_cast<uint8_t>(2 * hero.getLevel()));
      }
      else
        monster.takeDamage(hero.getDamageOutputVersus(monster), hero.damageType());
      applyLifeSteal(hero, monster, monsterHPBefore);
      if (!monster.isDefeated() && hero.has(HeroStatus::Poisonous))
      {
        const auto poisonAmount = hero.getIntensity(HeroStatus::Poisonous) * hero.getLevel();
        if (monster.poison(poisonAmount))
          appliedPoison = true;
      }
      if (hero.has(HeroStatus::CorrosiveStrike))
        monster.corrode(hero.getIntensity(HeroStatus::CorrosiveStrike));
      if (hero.has(HeroStatus::Might))
        monster.erodeResitances();
      hero.removeOneTimeAttackEffects();
    };

    auto monsterAttacks = [&] {
      if (!hero.tryDodge(allMonsters))
      {
        if (willPetrify)
        {
          // Hero either dies or death protection is triggered
          hero.loseHitPointsOutsideOfFight(hero.getHitPoints(), allMonsters);
          heroReceivedHit = true;
        }
        else
        {
          heroReceivedHit = hero.takeDamage(monsterDamageInitial, monster.damageType(), allMonsters);
          if (heroReceivedHit && monster.has(MonsterTrait::CurseBearer))
            hero.add(HeroDebuff::Cursed, allMonsters);
        }
        if (hero.has(HeroTrait::ManaShield) && heroReceivedHit && !hero.isDefeated())
          monster.takeManaShieldDamage(hero.getLevel());
      }
    };

    auto regularAttackSequence = [&] {
      if (hero.hasInitiativeVersus(monster))
      {
        heroAttacks();
        if (!monster.isDefeated())
        {
          // If the monster is defeated beyond this point (Reflexes or Mana Shield),
          // it was not slowed nor burning (except if attacked with Burning Strike) before the final blow.
          monsterWasSlowed = false;
          monsterWasBurning = hero.has(HeroStatus::BurningStrike);
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
    };

    PietyCollection collectPiety{hero};

    if (swiftHand)
    {
      monster.takeDamage(monster.getHitPoints(), DamageType::Typeless);
      monster.die();
    }
    else if (heroUsesDeathGaze)
      monster.takeDamage(hero.getBaseDamage(), DamageType::Typeless);

    if (!monster.isDefeated())
      regularAttackSequence();

    if (heroReceivedHit)
      detail::applyHitSideEffects(hero, monster);

    if (appliedPoison)
    {
      // Applying poison twice (with reflexes) does not trigger likes/dislikes twice
      collectPiety(hero.getFaith().monsterPoisoned(monster));
    }

    hero.adjustMomentum(monster.isDefeated());

    if (willPetrify && hero.isDefeated())
      return Summary::Petrified;

    return detail::finalizeAttack(hero, monster, monsterWasSlowed, monsterWasBurning, true, allMonsters, resources);
  }

  Summary
  attackWithKnockback(Hero& hero, Monster& primary, Monsters& allMonsters, Knockback knockback, Resources& resources)
  {
    const bool reflexes = hero.has(HeroStatus::Reflexes);
    auto summary = attack(hero, primary, allMonsters, resources);
    if (summary == Summary::Safe && hero.has(HeroStatus::Knockback))
    {
      switch (knockback.targetType)
      {
      case Knockback::TargetType::Empty:
      case Knockback::TargetType::Indestructible:
        break;
      case Knockback::TargetType::Monster:
        summary = knockBackMonster(hero, primary, allMonsters, knockback.monster, resources);
        if (reflexes && summary == Summary::Safe)
          summary = knockBackMonster(hero, primary, allMonsters, knockback.monster, resources);
        break;
      case Knockback::TargetType::Wall:
        if (!primary.has(MonsterTrait::Cowardly))
        {
          auto& resourceSet = resources();
          assert(resourceSet.numWalls > 0);
          summary = knockBackMonster(hero, primary, allMonsters, nullptr, resources);
          --resourceSet.numWalls;
          if (reflexes && summary == Summary::Safe && resourceSet.numWalls > 0)
          {
            summary = knockBackMonster(hero, primary, allMonsters, nullptr, resources);
            --resourceSet.numWalls;
          }
        }
        break;
      }
    }
    return summary;
  }
} // namespace Combat
