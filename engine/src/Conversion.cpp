#include "Conversion.hpp"

#include "Hero.hpp"
#include "HeroClass.hpp"
#include "HeroStatus.hpp"
#include "Items.hpp"

#include <cassert>

Conversion::Conversion(HeroClass theClass, HeroRace race)
  : points(0)
  , threshold(100)
{
  if (isMonsterClass(theClass))
  {
    switch (theClass)
    {
    case HeroClass::Vampire:
      threshold = 120;
      bonus = [](Hero& hero) { hero.addStatus(HeroStatus::LifeSteal); };
      break;
    case HeroClass::HalfDragon:
      threshold = 120;
      bonus = [](Hero& hero) { hero.addStatus(HeroStatus::Knockback, 20); };
      break;
    case HeroClass::Gorgon:
      threshold = 100;
      bonus = [](Hero& hero) { hero.addStatus(HeroStatus::DeathGaze, 5); };
      break;
    case HeroClass::RatMonarch:
      threshold = 80;
      bonus = [](Hero& hero) { hero.addStatus(HeroStatus::CorrosiveStrike); };
      break;
    case HeroClass::Goatperson:
      threshold = 100;
      bonus = [&](Hero& hero) {
        threshold += 10;
        hero.fullHealthAndMana();
      };
      break;
    default:
      assert(false);
    }
    return;
  }

  switch (race)
  {
  case HeroRace::Human:
    threshold = 100;
    bonus = [](Hero& hero) { hero.addDamageBonus(); };
    break;
  case HeroRace::Elf:
    threshold = 70;
    bonus = [](Hero& hero) { hero.addManaBonus(); };
    break;
  case HeroRace::Dwarf:
    threshold = 80;
    bonus = [](Hero& hero) { hero.addHealthBonus(); };
    break;
  case HeroRace::Halfling:
    threshold = 80;
    bonus = [](Hero& hero) { hero.receive(Item::HealthPotion); };
    break;
  case HeroRace::Gnome:
    threshold = 90;
    bonus = [](Hero& hero) { hero.receive(Item::ManaPotion); };
    break;
  case HeroRace::Orc:
    threshold = 80;
    bonus = [](Hero& hero) { hero.changeBaseDamage(+2); };
    break;
  case HeroRace::Goblin:
    threshold = 85;
    bonus = [xp = 5](Hero& hero) mutable {
      hero.gainExperience(xp);
      ++xp;
    };
    break;
  };
}

int Conversion::getPoints() const
{
  return points;
}

int Conversion::getThreshold() const
{
  return threshold;
}

bool Conversion::addPoints(int pointsAdded)
{
  points += pointsAdded;
  return points >= getThreshold();
}

void Conversion::applyBonus(Hero& hero)
{
  while (points >= getThreshold())
  {
    points -= getThreshold();
    bonus(hero);
  }
}
