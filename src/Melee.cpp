#include "Melee.hpp"

namespace Melee
{
  Outcome predictOutcome(const Hero& hero, const Monster& monster)
  {
    Outcome::Summary summary = Outcome::Summary::Error;
    Hero heroAfterFight(hero);
    Monster monsterAfterFight(monster);

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
      monsterAfterFight.takeDamage(hero.getDamage(), hero.hasStatus(HeroStatus::MagicalAttack));
      if (monsterAfterFight.isDefeated())
        summary = Outcome::Summary::HeroWins;
      else
      {
        heroAfterFight.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
        if (heroAfterFight.isDefeated())
          summary = Outcome::Summary::HeroDefeated;
        else
        {
          if (monster.bearsCurse())
            heroAfterFight.addStatus(HeroStatus::Cursed);
          if (hero.hasStatus(HeroStatus::Reflexes))
            monsterAfterFight.takeDamage(hero.getDamage(), hero.hasStatus(HeroStatus::MagicalAttack));
          summary = monsterAfterFight.isDefeated() ? Outcome::Summary::HeroWins : Outcome::Summary::Safe;
        }
      }
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
