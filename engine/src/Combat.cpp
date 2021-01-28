#include "Combat.hpp"

#include "Items.hpp"

#include <cassert>
#include <iostream>

namespace Combat
{
  namespace
  {
    void inflictDebuffs(Hero& hero, const Monster& monster, bool includeCurse)
    {
      if (monster.isPoisonous())
        hero.addStatus(HeroStatus::Poisoned);
      if (monster.hasManaBurn())
        hero.addStatus(HeroStatus::ManaBurned);
      if (monster.isCorrosive())
        hero.addStatus(HeroStatus::Corrosion);
      if (monster.isWeakening())
        hero.addStatus(HeroStatus::Weakened);
      if (includeCurse && monster.bearsCurse())
        hero.addStatus(HeroStatus::Cursed);
    }

    void applyLifeSteal(Hero& hero, const Monster& monster, int monsterHitPointsBefore)
    {
      if (hero.hasStatus(HeroStatus::LifeSteal) && !monster.isBloodless())
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

    Summary attack(Hero& hero, Monster& monster)
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

      hero.startPietyCollection();

      // TODO Better handling of dodge and dodge prediction -- manual selection?

      // TODO Handle knockback?
      // TODO Handle burn stack pop on other monsters?

      // Bonus experience is added if the monster was slowed before the final attack
      bool monsterWasSlowed = monster.isSlowed();
      bool monsterWasBurning = monster.isBurning();
      bool heroReceivedHit = false;

      const bool swiftHand = hero.hasTrait(HeroTrait::SwiftHand) && hero.getLevel() > monster.getLevel();
      const bool willPetrify = !hero.hasStatus(HeroStatus::DeathGazeImmune) &&
                               (monster.getDeathGazePercent() * hero.getHitPointsMax() > hero.getHitPoints() * 100);

      if (swiftHand)
      {
        monster.die();
      }
      else if (hero.hasInitiativeVersus(monster))
      {
        const int monsterDamageInitial = monster.getDamage();
        const int monsterHPBefore = monster.getHitPoints();
        if (hero.hasStatus(HeroStatus::CrushingBlow))
          monster.receiveCrushingBlow();
        else if (hero.hasStatus(HeroStatus::BurningStrike))
          monster.takeBurningStrikeDamage(hero.getDamageOutputVersus(monster), hero.getLevel(),
                                          hero.doesMagicalDamage());
        else
          monster.takeDamage(hero.getDamageOutputVersus(monster), hero.doesMagicalDamage());
        applyLifeSteal(hero, monster, monsterHPBefore);
        if (hero.hasStatus(HeroStatus::Poisonous) && !monster.isDefeated())
        {
          if (monster.poison(hero.getStatusIntensity(HeroStatus::Poisonous) * hero.getLevel()))
            hero.collect(hero.getFaith().monsterPoisoned(monster));
        }
        if (hero.hasStatus(HeroStatus::CorrosiveStrike))
          monster.corrode(hero.getStatusIntensity(HeroStatus::CorrosiveStrike));
        if (hero.hasStatus(HeroStatus::Might))
          monster.erodeResitances();
        if (!monster.isDefeated())
        {
          // If monster is defeated beyond this point, it was not slowed before the final blow
          monsterWasSlowed = false;
          monsterWasBurning = hero.hasStatus(HeroStatus::BurningStrike);
          if (!hero.tryDodge())
          {
            if (monster.bearsCurse())
              hero.addStatus(HeroStatus::Cursed);
            if (willPetrify)
            {
              // Hero either dies or death protection is triggered
              hero.loseHitPointsOutsideOfFight(hero.getHitPoints());
            }
            else
              hero.takeDamage(monsterDamageInitial, monster.doesMagicalDamage());
            heroReceivedHit = true;
          }
          if (!hero.isDefeated())
          {
            if (heroReceivedHit && hero.hasTrait(HeroTrait::ManaShield))
              monster.takeManaShieldDamage(hero.getLevel());
            if (hero.hasStatus(HeroStatus::Reflexes) && !monster.isDefeated())
            {
              hero.removeOneTimeAttackEffects();
              const int monsterHPBefore = monster.getHitPoints();
              if (hero.hasStatus(HeroStatus::BurningStrike))
                monster.takeBurningStrikeDamage(hero.getDamageOutputVersus(monster), hero.getLevel(),
                                                hero.doesMagicalDamage());
              else
                monster.takeDamage(hero.getDamageOutputVersus(monster), hero.doesMagicalDamage());
              applyLifeSteal(hero, monster, monsterHPBefore);
              if (hero.hasStatus(HeroStatus::Poisonous))
                monster.poison(hero.getStatusIntensity(HeroStatus::Poisonous) * hero.getLevel());
              if (hero.hasStatus(HeroStatus::CorrosiveStrike))
                monster.corrode(hero.getStatusIntensity(HeroStatus::CorrosiveStrike));
            }
          }
        }
      }
      else
      {
        assert(!hero.hasStatus(HeroStatus::Reflexes));
        if (!hero.tryDodge())
        {
          if (monster.bearsCurse())
            hero.addStatus(HeroStatus::Cursed);
          if (willPetrify)
          {
            // Hero either dies or death protection is triggered
            hero.loseHitPointsOutsideOfFight(hero.getHitPoints());
          }
          else
            hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
          heroReceivedHit = true;
        }
        if (!hero.isDefeated())
        {
          if (heroReceivedHit && hero.hasTrait(HeroTrait::ManaShield))
            monster.takeManaShieldDamage(hero.getLevel());
          const int monsterHPBefore = monster.getHitPoints();
          if (hero.hasStatus(HeroStatus::CrushingBlow))
            monster.receiveCrushingBlow();
          else if (hero.hasStatus(HeroStatus::BurningStrike))
            monster.takeBurningStrikeDamage(hero.getDamageOutputVersus(monster), hero.getLevel(),
                                            hero.doesMagicalDamage());
          else
            monster.takeDamage(hero.getDamageOutputVersus(monster), hero.doesMagicalDamage());
          applyLifeSteal(hero, monster, monsterHPBefore);
          if (hero.hasStatus(HeroStatus::Poisonous) && !monster.isDefeated())
          {
            if (monster.poison(hero.getStatusIntensity(HeroStatus::Poisonous) * hero.getLevel()))
              hero.collect(hero.getFaith().monsterPoisoned(monster));
          }
          if (hero.hasStatus(HeroStatus::CorrosiveStrike))
            monster.corrode(hero.getStatusIntensity(HeroStatus::CorrosiveStrike));
          if (hero.hasStatus(HeroStatus::Might))
            monster.erodeResitances();
        }
      }

      if (heroReceivedHit)
      {
        inflictDebuffs(hero, monster, false);
        hero.collect(hero.getFaith().receivedHit(monster));
      }

      // TODO: Check if attacks on plants count for momentum
      hero.adjustMomentum(monster.isDefeated());

      const auto summary = detail::summaryAndExperience(hero, monster, monsterWasSlowed, monsterWasBurning);
      hero.removeOneTimeAttackEffects();
      hero.applyCollectedPiety();
      return summary;
    }

    void attackedOther(Hero& hero, Monster& monster)
    {
      if (!monster.isBurning())
        return;
      const bool monsterWasSlowed = monster.isSlowed();
      monster.burnDown();
      if (monster.isDefeated())
        hero.monsterKilled(monster, monsterWasSlowed, true);
    }
  } // namespace

  // Perform melee attack on monster, evaluate effects on all monsters
  Summary attack(Hero& hero, Monster& monster, Monsters& monsters)
  {
    const int levelBefore = hero.getLevel() + hero.getPrestige();
    auto summary = attack(hero, monster);
    if (!hero.isDefeated())
    {
      for (auto& otherMonster : monsters)
      {
        if (otherMonster != monster)
          attackedOther(hero, otherMonster);
      }
      if (hero.getLevel() + hero.getPrestige() > levelBefore)
        summary = Summary::LevelUp;
    }
    return summary;
  }

  namespace detail
  {
    Summary summaryAndExperience(Hero& hero, const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning)
    {
      if (hero.isDefeated())
      {
        if (monster.getDeathGazePercent() == 0)
          return Summary::Death;
        return Summary::Petrified;
      }

      if (!monster.isDefeated())
        return Summary::Safe;

      const int levelBefore = hero.getLevel() + hero.getPrestige();
      hero.monsterKilled(monster, monsterWasSlowed, monsterWasBurning);
      if (hero.getLevel() + hero.getPrestige() > levelBefore)
        return Summary::LevelUp;
      return Summary::Win;
    }
  } // namespace detail

} // namespace Combat
