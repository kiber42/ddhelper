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
      if (includeCurse && monster.bearsCurse())
      {
        hero.addStatus(HeroStatus::Cursed);
        debuffs.insert(Debuff::Cursed);
      }

      return debuffs;
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

    bool monsterWasStunned = monster.isSlowed();

    if (hero.hasInitiativeVersus(monster))
    {
      // Damage is almost always computed using the initial hero and monster
      // Known exceptions:
      //   - Health from Life Steal is added directly after strike
      //   - Warlord's 30% damage bonus if hero's health is below 50%
      //   - A Curse Bearer monster will curse the hero directly after his strike
      outcome.monster.takeDamage(hero.getDamageVersus(monster), hero.doesMagicalDamage());
      if (!outcome.monster.isDefeated())
      {
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
        outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
        if (hero.hasStatus(HeroStatus::Reflexes) && !outcome.hero.isDefeated())
        {
          monsterWasStunned = false;
          outcome.monster.takeDamage(hero.getDamageVersus(monster), hero.doesMagicalDamage());
        }
      }
    }
    else
    {
      assert(!hero.hasStatus(HeroStatus::Reflexes));
      outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
      if (!outcome.hero.isDefeated())
      {
        outcome.monster.takeDamage(hero.getDamageVersus(monster), hero.doesMagicalDamage());
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
      }
    }

    outcome.debuffs = receiveDebuffs(outcome.hero, monster, false);
    if (hero.hasStatus(HeroStatus::DeathProtection) && !outcome.hero.hasStatus(HeroStatus::DeathProtection))
      outcome.debuffs.insert(Outcome::Debuff::LostDeathProtection);
    if (outcome.monster.isDefeated())
    {
      if (monster.bearsCurse())
        outcome.hero.addStatus(HeroStatus::Cursed);
      else
        outcome.hero.removeStatus(HeroStatus::Cursed, false);
    }

    outcome.hero.removeStatus(HeroStatus::ConsecratedStrike, true);
    // if (!hero.hasTrait(HeroTrait::Dextrous))
    outcome.hero.removeStatus(HeroStatus::FirstStrike, true);
    outcome.hero.removeStatus(HeroStatus::Reflexes, true);

    outcome.summary = summaryAndExperience(outcome.hero, outcome.monster, monster.isSlowed());

    return outcome;
  }

  Outcome::Debuffs retaliate(Hero& hero, const Monster& monster)
  {
    const bool deathProtectionBefore = hero.hasStatus(HeroStatus::DeathProtection);
    hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
    auto debuffs = receiveDebuffs(hero, monster, true);
    if (deathProtectionBefore && !hero.hasStatus(HeroStatus::DeathProtection))
      debuffs.insert(Outcome::Debuff::LostDeathProtection);
    return debuffs;
  }

  Outcome attackOther(const Hero& hero, const Monster& monster)
  {
    Outcome outcome{Outcome::Summary::Safe, {}, hero, monster};
    outcome.monster.burnDown();
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
