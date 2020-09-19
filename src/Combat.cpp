#include "Combat.hpp"

#include <cassert>
#include <iostream>

namespace Combat
{
  namespace
  {
    Outcome::Debuffs receiveDebuffs(Hero& hero, const Monster& monster, bool includeCurse)
    {
      using Debuff = Outcome::Debuff;
      Outcome::Debuffs debuffs;

      if (monster.isPoisonous() && !hero.hasStatus(HeroStatus::Poisoned))
      {
        hero.addStatus(HeroStatus::Poisoned);
        debuffs.insert(Debuff::Poisoned);
      }
      if (monster.hasManaBurn() && !hero.hasStatus(HeroStatus::ManaBurned))
      {
        hero.addStatus(HeroStatus::ManaBurned);
        debuffs.insert(Debuff::ManaBurned);
      }
      if (monster.isCorrosive())
      {
        hero.addStatus(HeroStatus::Corrosion);
        debuffs.insert(Debuff::Corroded);
      }
      if (monster.isWeakening())
      {
        hero.addStatus(HeroStatus::Weakened);
        debuffs.insert(Debuff::Weakened);
      }
      if (monster.bearsCurse())
      {
        if (includeCurse)
          hero.addStatus(HeroStatus::Cursed);
        debuffs.insert(Debuff::Cursed);
      }

      return debuffs;
    }

    // The "determined" trait may become active during combat, the effect is applied immediately
    int damageAccountingForDetermined(const Hero& before, const Hero& after, const Monster& monster)
    {
      const bool wasActive = before.isTraitActive(HeroTrait::Determined);
      const bool isActive = after.isTraitActive(HeroTrait::Determined);
      if (wasActive != isActive)
      {
        // It seems possible that the trait becomes disabled during fight (probably with Life Steal and Reflexes)
        Hero determinedHero = before;
        determinedHero.changeBaseDamage(isActive ? +30 : -30);
        return determinedHero.getDamageVersus(monster);
      }
      return before.getDamageVersus(monster);
    }
  } // namespace

  Outcome predictOutcome(const Hero& hero, const Monster& monster)
  {
    Outcome outcome{Outcome::Summary::NotPossible, {}, hero, monster};

    if (hero.isDefeated())
    {
      std::cerr << "Dead hero cannot fight." << std::endl;
      return outcome;
    }
    if (monster.isDefeated())
    {
      std::cerr << "Cannot fight defeated monster." << std::endl;
      return outcome;
    }

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

    const bool swiftHand = hero.hasTrait(HeroTrait::SwiftHand) && hero.getLevel() > monster.getLevel();
    const bool petrified = monster.getDeathGazePercent() * hero.getHitPointsMax() / 100 > hero.getHitPoints();

    if (swiftHand)
    {
      outcome.monster.die();
    }
    else if (petrified)
    {
      // Hero either dies or death protection is triggered
      outcome.hero.loseHitPointsOutsideOfFight(hero.getHitPoints());
    }
    else if (hero.hasInitiativeVersus(monster))
    {
      if (hero.hasStatus(HeroStatus::CrushingBlow))
        outcome.monster.receiveCrushingBlow();
      else
        outcome.monster.takeDamage(hero.getDamageVersus(monster), hero.doesMagicalDamage());
      if (hero.hasStatus(HeroStatus::Might))
        outcome.monster.erodeResitances();
      if (!outcome.monster.isDefeated())
      {
        // If monster is defeated beyond this point, it was not slowed before the final blow
        monsterWasSlowed = false;
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
        outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
        if (!outcome.hero.isDefeated())
        {
          if (hero.hasTrait(HeroTrait::ManaShield))
            outcome.monster.takeManaShieldDamage(hero.getLevel());
          if (hero.hasStatus(HeroStatus::Reflexes))
          {
            outcome.hero.removeOneTimeAttackEffects();
            outcome.monster.takeDamage(outcome.hero.getDamageVersus(outcome.monster), outcome.hero.doesMagicalDamage());
          }
        }
      }
    }
    else
    {
      assert(!hero.hasStatus(HeroStatus::Reflexes));
      outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
      if (!outcome.hero.isDefeated())
      {
        if (hero.hasTrait(HeroTrait::ManaShield))
          outcome.monster.takeManaShieldDamage(hero.getLevel());
        if (hero.hasStatus(HeroStatus::CrushingBlow))
          outcome.monster.receiveCrushingBlow();
        else
        {
          const int damage = damageAccountingForDetermined(hero, outcome.hero, monster);
          outcome.monster.takeDamage(damage, hero.doesMagicalDamage());
        }
        if (hero.hasStatus(HeroStatus::Might))
          outcome.monster.erodeResitances();
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
      }
    }

    if (!swiftHand)
      outcome.debuffs = receiveDebuffs(outcome.hero, monster, false);

    if (hero.hasStatus(HeroStatus::DeathProtection) && !outcome.hero.hasStatus(HeroStatus::DeathProtection))
      outcome.debuffs.insert(Outcome::Debuff::LostDeathProtection);
    if (outcome.monster.isDefeated())
    {
      if (monster.bearsCurse())
      {
        outcome.hero.addStatus(HeroStatus::Cursed);
        outcome.debuffs.insert(Outcome::Debuff::Cursed);
      }
      else
        outcome.hero.removeStatus(HeroStatus::Cursed, false);
    }

    outcome.hero.removeOneTimeAttackEffects();

    outcome.summary = petrified && outcome.hero.isDefeated()
                          ? Outcome::Summary::Petrified
                          : summaryAndExperience(outcome.hero, outcome.monster, monsterWasSlowed);

    return outcome;
  }

  Outcome::Debuffs retaliate(Hero& hero, Monster& monster)
  {
    const bool deathProtectionBefore = hero.hasStatus(HeroStatus::DeathProtection);
    hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
    if (hero.hasTrait(HeroTrait::ManaShield))
      monster.takeManaShieldDamage(hero.getLevel());
    auto debuffs = receiveDebuffs(hero, monster, true);
    if (deathProtectionBefore && !hero.hasStatus(HeroStatus::DeathProtection))
      debuffs.insert(Outcome::Debuff::LostDeathProtection);
    return debuffs;
  }

  Outcome attackOther(const Hero& hero, const Monster& monster)
  {
    Outcome outcome{Outcome::Summary::Safe, {}, hero, monster};
    outcome.monster.burnDown();
    outcome.hero.removeOneTimeAttackEffects();
    if (outcome.monster.isDefeated())
    {
      outcome.summary = summaryAndExperience(outcome.hero, outcome.monster, monster.isSlowed());
      if (outcome.monster.bearsCurse())
      {
        outcome.hero.addStatus(HeroStatus::Cursed);
        outcome.debuffs.insert(Outcome::Debuff::Cursed);
      }
      else
        outcome.hero.removeStatus(HeroStatus::Cursed, false);
    }
    return outcome;
  }

  Outcome uncoverTiles(const Hero& hero, const Monster& monster, int numTiles)
  {
    Outcome outcome{Outcome::Summary::Safe, {}, hero, monster};
    outcome.hero.recover(numTiles);
    outcome.monster.recover(numTiles);
    return outcome;
  }

  Hero uncoverTiles(Hero hero, int numTiles)
  {
    hero.recover(numTiles);
    return hero;
  }

  Outcome::Summary summaryAndExperience(Hero& heroAfterFight, const Monster& monsterAfterFight, bool monsterWasSlowed)
  {
    using Summary = Outcome::Summary;
    if (heroAfterFight.isDefeated())
      return Summary::Death;

    if (!monsterAfterFight.isDefeated())
      return Summary::Safe;

    const int levelInitial = heroAfterFight.getLevel();
    heroAfterFight.gainExperience(
        Experience::forHeroAndMonsterLevels(heroAfterFight.getLevel(), monsterAfterFight.getLevel()), monsterWasSlowed);
    return heroAfterFight.getLevel() > levelInitial ? Outcome::Summary::LevelUp : Outcome::Summary::Win;
  }

} // namespace Combat
