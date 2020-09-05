#include "Melee.hpp"

#include <cassert>
#include <iostream>

namespace Melee
{
  Outcome predictOutcome(const Hero& hero, const Monster& monster)
  {
    using Summary = Outcome::Summary;
    using Debuff = Outcome::Debuff;
    Outcome outcome{Summary::NotPossible, {}, hero, monster};

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
      if (!outcome.monster.isDefeated())
      {
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
        outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
        if (hero.hasStatus(HeroStatus::Reflexes) && !outcome.hero.isDefeated())
          outcome.monster.takeDamage(hero.getDamage(), hero.doesMagicalDamage());
      }
    }
    else
    {
      assert(!hero.hasStatus(HeroStatus::Reflexes));
      outcome.hero.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
      if (!outcome.hero.isDefeated())
      {
        outcome.monster.takeDamage(hero.getDamage(), hero.doesMagicalDamage());
        if (monster.bearsCurse())
          outcome.hero.addStatus(HeroStatus::Cursed);
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
    // if (!hero.hasTrait(HeroTrait::Dextrous))
    outcome.hero.removeStatus(HeroStatus::FirstStrike, true);
    outcome.hero.removeStatus(HeroStatus::Reflexes, true);

    if (outcome.hero.isDefeated())
      outcome.summary = Summary::Death;
    else
    {
      outcome.summary = outcome.monster.isDefeated() ? Summary::Win : Summary::Safe;
      auto flagDebuff = [&outcome, &hero](HeroStatus status, Debuff debuff) {
        if (outcome.hero.getStatusIntensity(status) > hero.getStatusIntensity(status))
          outcome.debuffs.insert(debuff);
      };
      flagDebuff(HeroStatus::Cursed, Debuff::Cursed);
      flagDebuff(HeroStatus::Poisoned, Debuff::Poisoned);
      flagDebuff(HeroStatus::ManaBurned, Debuff::ManaBurned);
      flagDebuff(HeroStatus::Corrosion, Debuff::Corroded);
      flagDebuff(HeroStatus::Weakened, Debuff::Weakened);
      if (outcome.hero.getDeathProtection() < hero.getDeathProtection())
        outcome.debuffs.insert(Debuff::LostDeathProtection);
    }

    return outcome;
  }
} // namespace Melee
