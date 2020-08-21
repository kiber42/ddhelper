#include "Melee.hpp"

namespace Melee
{
  Outcome predictOutcome(const Hero& hero, const Monster& monster)
  {
    Outcome::Summary summary = Outcome::Summary::Error;
    Hero heroAfterFight(hero);
    Monster monsterAfterFight(monster);

    // TODO Handle evasion

    // TODO Handle knock back

    if (hero.hasInitiativeVersus(monster))
    {
      // damage is always computed using the initial hero or monster!
      //   (is that correct? what about e.g. warlord's damage bonus at low health?)
      // TODO Handle attack with Flaming Sword
      monsterAfterFight.takeDamage(hero.getDamage(), hero.hasStatus(HeroStatus::MagicalAttack));
      if (monsterAfterFight.isDefeated())
        summary = Outcome::Summary::HeroWins;
      else
      {
        heroAfterFight.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
        if (heroAfterFight.isDefeated())
          summary = Outcome::Summary::HeroDefeated;
        else
          summary = Outcome::Summary::Safe;
        if (monster.bearsCurse())
          heroAfterFight.addStatus(HeroStatus::Cursed);
      }
      if (hero.hasStatus(HeroStatus::Reflexes))
        monsterAfterFight.takeDamage(hero.getDamage(), hero.hasStatus(HeroStatus::MagicalAttack));
    }
    else
    {
      heroAfterFight.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
      if (heroAfterFight.isDefeated())
        summary = Outcome::Summary::HeroDefeated;
      else
      {
        if (monster.bearsCurse())
          heroAfterFight.addStatus(HeroStatus::Cursed);
        monsterAfterFight.takeDamage(hero.getDamage(), hero.hasStatus(HeroStatus::MagicalAttack));
        if (monsterAfterFight.isDefeated())
          summary = Outcome::Summary::HeroWins;
        else
          summary = Outcome::Summary::Safe;
      }
    }

    if (monster.isPoisonous())
      heroAfterFight.addStatus(HeroStatus::Poisoned);
    if (monster.hasManaBurn())
      heroAfterFight.addStatus(HeroStatus::ManaBurn);
    if (monster.bearsCurse() && monsterAfterFight.isDefeated())
      heroAfterFight.addStatus(HeroStatus::Cursed);
    if (monster.isCorrosive())
      heroAfterFight.addStatus(HeroStatus::Corrosion);
    if (monster.isWeakening())
      heroAfterFight.addStatus(HeroStatus::Weakened);

    //  std::vector<HeroStatus> statusAdded;
    //  for (auto& status : {HeroStatus::Poisoned, HeroStatus::ManaBurn, HeroStatus::Cursed, HeroStatus::Corrosion,
    //  HeroStatus::Weakened})
    //  {
    //     if (!hero.hasStatus(status) && heroAfterFight.hasStatus(status))
    //       statusAdded.push_back(status);
    //  }

    return Outcome{summary, std::move(heroAfterFight), std::move(monsterAfterFight)};
  }
} // namespace Melee
