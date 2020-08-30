#include "Melee.hpp"

#include <cassert>

namespace Melee
{
  Outcome predictOutcome(const Hero& hero, const Monster& monster)
  {
    using Summary = Outcome::Summary;
    std::optional<Summary> summary;
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
        summary = Summary::HeroWins;
      else
      {
        if (monster.bearsCurse())
          heroAfterFight.addStatus(HeroStatus::Cursed);
        heroAfterFight.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
        if (heroAfterFight.isDefeated())
          summary = Summary::HeroDefeated;
        else
        {
          if (hero.hasStatus(HeroStatus::Reflexes))
            monsterAfterFight.takeDamage(hero.getDamage(), hero.hasStatus(HeroStatus::MagicalAttack));
          summary = monsterAfterFight.isDefeated() ? Summary::HeroWins : Summary::Safe;
        }
      }
    }
    else
    {
      assert(!hero.hasStatus(HeroStatus::Reflexes));
      heroAfterFight.takeDamage(monster.getDamage(), monster.doesMagicalDamage());
      if (heroAfterFight.isDefeated())
        summary = Summary::HeroDefeated;
      else
      {
        monsterAfterFight.takeDamage(hero.getDamage(), hero.hasStatus(HeroStatus::MagicalAttack));
        if (monster.bearsCurse())
          heroAfterFight.addStatus(HeroStatus::Cursed);
        summary = monsterAfterFight.isDefeated() ? Summary::HeroWins : Summary::Safe;
      }
    }

    if (monster.isPoisonous())
      heroAfterFight.addStatus(HeroStatus::Poisoned);
    if (monster.hasManaBurn())
      heroAfterFight.addStatus(HeroStatus::ManaBurn);
    if (monsterAfterFight.isDefeated())
    {
      if (monster.bearsCurse())
        heroAfterFight.addStatus(HeroStatus::Cursed);
      else
        heroAfterFight.removeStatus(HeroStatus::Cursed, false);
    }
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

    if (!summary.has_value())
      throw std::logic_error("Internal error in Melee::predict_outcome");

    return Outcome{summary.value(), std::move(heroAfterFight), std::move(monsterAfterFight)};
  }
} // namespace Melee
