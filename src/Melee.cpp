#include "Melee.hpp"

#include <cassert>
#include <iostream>

namespace Melee
{
  Outcome predictOutcome(const Hero& hero, const Monster& monster)
  {
    using Summary = Outcome::Summary;
    Outcome outcome{Summary::NotPossible, hero, monster};

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

    if (hero.hasInitiativeVersus(monster))
    {
      // Damage is almost always computed using the initial hero and monster
      // Known exceptions:
      //   - Health from Life Steal is added directly after strike
      //   - Warlord's 30% damage bonus if hero's health is below 50%
      //   - A Curse Bearer monster will curse the hero directly after his strike
      outcome.monster.takeDamage(hero.getDamage(), hero.doesMagicalDamage());
      if (outcome.monster.isDefeated())
        outcome.summary = Summary::HeroWins;
      else
      {
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
        outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
        if (outcome.hero.isDefeated())
          outcome.summary = Summary::HeroDefeated;
        else
        {
          if (hero.hasStatus(HeroStatus::Reflexes))
            outcome.monster.takeDamage(hero.getDamage(), hero.doesMagicalDamage());
          outcome.summary = outcome.monster.isDefeated() ? Summary::HeroWins : Summary::Safe;
        }
      }
    }
    else
    {
      assert(!hero.hasStatus(HeroStatus::Reflexes));
      outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
      if (outcome.hero.isDefeated())
        outcome.summary = Summary::HeroDefeated;
      else
      {
        outcome.monster.takeDamage(hero.getDamage(), hero.doesMagicalDamage());
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
        outcome.summary = outcome.monster.isDefeated() ? Summary::HeroWins : Summary::Safe;
      }
    }

    if (monster.isPoisonous())
      outcome.hero.addStatus(HeroStatus::Poisoned);
    if (monster.hasManaBurn())
      outcome.hero.addStatus(HeroStatus::ManaBurned);
    if (outcome.monster.isDefeated())
    {
      if (monster.bearsCurse())
        outcome.hero.addStatus(HeroStatus::Cursed);
      else
        outcome.hero.removeStatus(HeroStatus::Cursed, false);
    }
    if (monster.isCorrosive())
      outcome.hero.addStatus(HeroStatus::Corrosion);
    if (monster.isWeakening())
      outcome.hero.addStatus(HeroStatus::Weakened);

    outcome.hero.removeStatus(HeroStatus::ConsecratedStrike, true);
    outcome.hero.removeStatus(HeroStatus::FirstStrike, true);
    outcome.hero.removeStatus(HeroStatus::Reflexes, true);

    //  std::vector<HeroStatus> statusAdded;
    //  for (auto& status : {HeroStatus::Poisoned, HeroStatus::ManaBurn, HeroStatus::Cursed, HeroStatus::Corrosion,
    //  HeroStatus::Weakened})
    //  {
    //     if (!hero.hasStatus(status) && outcome.hero.hasStatus(status))
    //       statusAdded.push_back(status);
    //  }

    return outcome;
  }
} // namespace Melee
