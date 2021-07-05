#include "engine/Conversion.hpp"

#include "engine/Hero.hpp"
#include "engine/Items.hpp"

#include <cassert>

Conversion::Conversion(DungeonSetup setup)
  : points(0)
  , threshold(100)
{
  if (isMonsterClass(setup.heroClass))
  {
    switch (setup.heroClass)
    {
    case HeroClass::Vampire:
      threshold = 120;
      bonus = [](Hero& hero, Monsters&) { hero.add(HeroStatus::LifeSteal); };
      break;
    case HeroClass::HalfDragon:
      threshold = 100;
      bonus = [](Hero& hero, Monsters&) { hero.add(HeroStatus::Knockback, 20); };
      break;
    case HeroClass::Gorgon:
      threshold = 100;
      bonus = [](Hero& hero, Monsters&) { hero.add(HeroStatus::DeathGaze, 5); };
      break;
    case HeroClass::RatMonarch:
      threshold = 80;
      bonus = [](Hero& hero, Monsters&) { hero.add(HeroStatus::CorrosiveStrike); };
      break;
    case HeroClass::Goatperson:
      threshold = 100;
      bonus = [&](Hero& hero, Monsters&) {
        threshold += 10;
        hero.refillHealthAndMana();
      };
      break;
    default:
      assert(false);
    }
    return;
  }

  switch (setup.heroRace)
  {
  case HeroRace::Human:
    threshold = 100;
    bonus = [](Hero& hero, Monsters&) { hero.addAttackBonus(); };
    break;
  case HeroRace::Elf:
    threshold = 70;
    bonus = [](Hero& hero, Monsters&) {
      hero.addManaBonus();
      if (hero.has(HeroTrait::SpiritSword))
        hero.recoverManaPoints(1);
    };
    break;
  case HeroRace::Dwarf:
    threshold = 80;
    bonus = [](Hero& hero, Monsters&) { hero.addHealthBonus(); };
    break;
  case HeroRace::Halfling:
    threshold = 80;
    bonus = [](Hero& hero, Monsters&) { hero.receive(Potion::HealthPotion); };
    break;
  case HeroRace::Gnome:
    threshold = 90;
    bonus = [](Hero& hero, Monsters&) { hero.receive(Potion::ManaPotion); };
    break;
  case HeroRace::Orc:
    threshold = 80;
    bonus = [](Hero& hero, Monsters&) { hero.changeBaseDamage(+2); };
    break;
  case HeroRace::Goblin:
    threshold = 85;
    bonus = [xp = 5u](Hero& hero, Monsters& allMonsters) mutable {
      hero.gainExperienceNoBonuses(xp, allMonsters);
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

bool Conversion::addPoints(unsigned pointsAdded)
{
  points += pointsAdded;
  return points >= getThreshold();
}

void Conversion::applyBonus(Hero& hero, Monsters& allMonsters)
{
  while (points >= getThreshold())
  {
    points -= getThreshold();
    bonus(hero, allMonsters);
    if (hero.has(HeroTrait::SpiritSword))
      hero.addSpiritStrength();
  }
}
