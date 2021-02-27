#include "Combat.hpp"

#include "Items.hpp"

#include <cassert>
#include <iostream>

namespace Combat
{
  namespace
  {
    void inflictDebuffs(Hero& hero, const Monster& monster, bool includeCurse, Monsters& allMonsters)
    {
      if (monster.isPoisonous())
        hero.addStatus(HeroDebuff::Poisoned, allMonsters);
      if (monster.hasManaBurn())
        hero.addStatus(HeroDebuff::ManaBurned, allMonsters);
      if (monster.isCorrosive())
        hero.addStatus(HeroDebuff::Corroded, allMonsters);
      if (monster.isWeakening())
        hero.addStatus(HeroDebuff::Weakened, allMonsters);
      if (includeCurse && monster.bearsCurse())
        hero.addStatus(HeroDebuff::Cursed, allMonsters);
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
  } // namespace

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

    // TODO Better handling of dodge and dodge prediction -- manual selection?

    // TODO Handle knockback?
    // TODO Handle burn stack pop on other monsters?

    // Bonus experience is added if the monster was slowed before the final attack
    bool monsterWasSlowed = monster.isSlowed();
    bool monsterWasBurning = monster.isBurning();
    const int monsterDamageInitial = monster.getDamage();
    bool heroReceivedHit = false;
    bool appliedPoison = false;

    const bool reflexes = hero.hasStatus(HeroStatus::Reflexes);
    const bool swiftHand = hero.hasTrait(HeroTrait::SwiftHand) && hero.getLevel() > monster.getLevel();
    const bool willPetrify = !hero.hasStatus(HeroStatus::DeathGazeImmune) &&
                             (monster.getDeathGazePercent() * hero.getHitPointsMax() > hero.getHitPoints() * 100);

    auto heroAttacks = [&] {
      const int monsterHPBefore = monster.getHitPoints();
      if (hero.hasStatus(HeroStatus::CrushingBlow))
        monster.receiveCrushingBlow();
      else if (hero.hasStatus(HeroStatus::BurningStrike))
        monster.takeBurningStrikeDamage(hero.getDamageOutputVersus(monster), hero.getLevel(), hero.doesMagicalDamage());
      else
        monster.takeDamage(hero.getDamageOutputVersus(monster), hero.doesMagicalDamage());
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
        if (monster.bearsCurse())
          hero.addStatus(HeroDebuff::Cursed, allMonsters);
        if (willPetrify)
        {
          // Hero either dies or death protection is triggered
          hero.loseHitPointsOutsideOfFight(hero.getHitPoints(), allMonsters);
        }
        else
          hero.takeDamage(monsterDamageInitial, monster.doesMagicalDamage(), allMonsters);
        heroReceivedHit = true;
        if (!hero.isDefeated() && hero.hasTrait(HeroTrait::ManaShield))
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
        // it was not slowed nor burning before the final blow.
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
      inflictDebuffs(hero, monster, false, allMonsters);
      hero.collect(hero.getFaith().receivedHit(monster));
    }

    if (appliedPoison)
    {
      // Applying poison twice (with reflexes) does not trigger likes/dislikes twice
      hero.collect(hero.getFaith().monsterPoisoned(monster));
    }

    hero.adjustMomentum(monster.isDefeated());

    return detail::finalizeAttack(hero, monster, monsterWasSlowed, monsterWasBurning, true, allMonsters);
  }

  namespace detail
  {
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

    // Determines outcome summary and awards experience if applicable.
    // Helper used by Combat::attack and Magic::cast, do not call directly.
    Summary summaryAndExperience(
        Hero& hero, const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning, Monsters& allMonsters)
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
      hero.monsterKilled(monster, monsterWasSlowed, monsterWasBurning, allMonsters);
      if (hero.getLevel() + hero.getPrestige() > levelBefore)
        return Summary::LevelUp;
      return Summary::Win;
    }

    Summary finalizeAttack(Hero& hero,
                           const Monster& monster,
                           bool monsterWasSlowed,
                           bool monsterWasBurning,
                           bool triggerBurndown,
                           Monsters& allMonsters)
    {
      auto summary = detail::summaryAndExperience(hero, monster, monsterWasSlowed, monsterWasBurning, allMonsters);

      if (!hero.isDefeated() && triggerBurndown)
      {
        const int levelBefore = hero.getLevel() + hero.getPrestige();
        for (auto& otherMonster : allMonsters)
        {
          if (otherMonster != monster)
            detail::attackedOther(hero, otherMonster, allMonsters);
        }
        if (hero.getLevel() + hero.getPrestige() > levelBefore)
          summary = Summary::LevelUp;
      }

      hero.applyCollectedPiety(allMonsters);

      return summary;
    }
  } // namespace detail

} // namespace Combat
