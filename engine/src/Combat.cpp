#include "Combat.hpp"

#include <cassert>
#include <iostream>

namespace Combat
{
  namespace
  {
    void inflictDebuffs(Hero& hero, const Monster& monster, bool includeCurse)
    {
      if (monster.isPoisonous() && !hero.hasStatus(HeroStatus::Poisoned))
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
  } // namespace

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

    // TODO Handle attack with Flaming Sword
    // TODO Handle evasion?
    // TODO Handle knockback?
    // TODO Handle burn stack pop on other monsters?

    // Damage is almost always computed using the initial hero and monster
    // Known exceptions:
    //   - Health from Life Steal is added directly after strike
    //   - Warlord's 30% damage bonus if hero's health is below 50%
    //   - A Curse Bearer monster will curse the hero directly after the hero's strike

    // Bonus experience is added if the monster was slowed before the attack
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
      if (hero.hasStatus(HeroStatus::CrushingBlow))
        monster.receiveCrushingBlow();
      else
        monster.takeDamage(hero.getDamageVersus(monster), hero.doesMagicalDamage());
      if (hero.hasStatus(HeroStatus::Poisonous))
      {
        if (monster.poison(hero.getStatusIntensity(HeroStatus::Poisonous)))
          hero.collect(hero.getFaith().monsterPoisoned(monster));
      }
      if (hero.hasStatus(HeroStatus::Might))
        monster.erodeResitances();
      if (!monster.isDefeated())
      {
        // If monster is defeated beyond this point, it was not slowed before the final blow
        monsterWasSlowed = false;
        monsterWasBurning = false; // TODO: Account for burning strike
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
        if (!hero.isDefeated())
        {
          if (hero.hasTrait(HeroTrait::ManaShield))
            monster.takeManaShieldDamage(hero.getLevel());
          if (hero.hasStatus(HeroStatus::Reflexes))
          {
            hero.removeOneTimeAttackEffects();
            monster.takeDamage(hero.getDamageVersus(monster), hero.doesMagicalDamage());
            if (hero.hasStatus(HeroStatus::Poisonous))
              monster.poison(hero.getStatusIntensity(HeroStatus::Poisonous));
          }
        }
      }
    }
    else
    {
      assert(!hero.hasStatus(HeroStatus::Reflexes));
      if (willPetrify)
      {
        // Hero either dies or death protection is triggered
        hero.loseHitPointsOutsideOfFight(hero.getHitPoints());
      }
      else
        hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
      heroReceivedHit = true;
      if (!hero.isDefeated())
      {
        if (hero.hasTrait(HeroTrait::ManaShield))
          monster.takeManaShieldDamage(hero.getLevel());
        if (hero.hasStatus(HeroStatus::CrushingBlow))
          monster.receiveCrushingBlow();
        else
          monster.takeDamage(hero.getDamageVersus(monster), hero.doesMagicalDamage());
        if (hero.hasStatus(HeroStatus::Poisonous))
        {
          if (monster.poison(hero.getStatusIntensity(HeroStatus::Poisonous)))
            hero.collect(hero.getFaith().monsterPoisoned(monster));
        }
        if (hero.hasStatus(HeroStatus::Might))
          monster.erodeResitances();
        if (monster.bearsCurse())
          hero.addStatus(HeroStatus::Cursed);
      }
    }

    if (heroReceivedHit)
    {
      inflictDebuffs(hero, monster, false);
      hero.collect(hero.getFaith().receivedHit(monster));
    }

    if (monster.isDefeated())
      hero.collect(hero.getFaith().monsterKilled(monster, hero.getLevel(), monsterWasBurning));

    hero.removeOneTimeAttackEffects();
    detail::monsterDefeatedCurse(hero, monster);
    const auto summary = detail::summaryAndExperience(hero, monster, monsterWasSlowed);
    hero.applyCollectedPiety();
    return summary;
  }

  Summary attackOther(Hero& hero, Monster& monster)
  {
    const bool monsterWasSlowed = monster.isSlowed();
    monster.burnDown();
    hero.removeOneTimeAttackEffects();
    detail::monsterDefeatedCurse(hero, monster);
    return detail::summaryAndExperience(hero, monster, monsterWasSlowed);
  }

  Summary uncoverTiles(Hero& hero, Monster* monster, int numTiles)
  {
    hero.recover(numTiles);
    if (monster)
      monster->recover(numTiles);
    return Summary::Safe;
  }

  Debuffs findDebuffs(const Hero& before, const Hero& after)
  {
    Debuffs debuffs;
    if (before.hasStatus(HeroStatus::DeathProtection) && !after.hasStatus(HeroStatus::DeathProtection))
      debuffs.insert(Debuff::LostDeathProtection);
    if (before.getStatusIntensity(HeroStatus::Cursed) < after.getStatusIntensity(HeroStatus::Cursed))
      debuffs.insert(Debuff::Cursed);
    if (before.hasStatus(HeroStatus::ManaBurned) && !after.hasStatus(HeroStatus::ManaBurned))
      debuffs.insert(Debuff::ManaBurned);
    if (before.hasStatus(HeroStatus::Poisoned) && !after.hasStatus(HeroStatus::Poisoned))
      debuffs.insert(Debuff::Poisoned);
    if (before.getStatusIntensity(HeroStatus::Corrosion) < after.getStatusIntensity(HeroStatus::Corrosion))
      debuffs.insert(Debuff::Corroded);
    if (before.getStatusIntensity(HeroStatus::Weakened) < after.getStatusIntensity(HeroStatus::Weakened))
      debuffs.insert(Debuff::Weakened);
    return debuffs;
  }

  namespace detail
  {
    Summary summaryAndExperience(Hero& hero, const Monster& monster, bool monsterWasSlowed)
    {
      if (hero.isDefeated())
      {
        if (monster.getDeathGazePercent() == 0)
          return Summary::Death;
        return Summary::Petrified;
      }

      if (!monster.isDefeated())
        return Summary::Safe;

      const int levelBefore = hero.getLevel();
      hero.gainExperience(Experience::forHeroAndMonsterLevels(levelBefore, monster.getLevel()), monsterWasSlowed);

      if (hero.getLevel() > levelBefore)
        return Summary::LevelUp;
      return Summary::Win;
    }

    void monsterDefeatedCurse(Hero& hero, const Monster& monster)
    {
      if (monster.isDefeated())
      {
        if (monster.bearsCurse())
          hero.addStatus(HeroStatus::Cursed);
        else
          hero.removeStatus(HeroStatus::Cursed, false);
      }
    }
  } // namespace detail

} // namespace Combat
