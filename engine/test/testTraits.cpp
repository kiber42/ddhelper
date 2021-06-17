#include "bandit/bandit.h"

#include "engine/Combat.hpp"
#include "engine/Hero.hpp"
#include "engine/Monster.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
}

void testHeroTraits()
{
  describe("Elf", [] {
    it("should receive one max mana point on conversion", [] {
      Hero hero(HeroClass::Guard, HeroRace::Elf);
      hero.addConversionPoints(70, noOtherMonsters);
      AssertThat(hero.getManaPointsMax(), Equals(11));
      AssertThat(hero.getManaPoints(), Equals(10));
    });
  });
  describe("Spirit Sword", [] {
    it("should apply spirit strength on conversion", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      const auto baseDamage = hero.getBaseDamage();
      hero.addTrait(HeroTrait::SpiritSword);
      hero.addConversionPoints(1000, noOtherMonsters);
      const auto spiritStrength = hero.getLevel() + hero.getManaPointsMax();
      AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(spiritStrength));
      AssertThat(hero.getManaPoints(), Equals(0));
      AssertThat(hero.getBaseDamage(), Equals(baseDamage + spiritStrength));
    });
    it("should work correctly for Transmuter Elves", [] {
      // Elf transmuter conversion threshold is a special case:
      // the freshly added mana point is counted for spirit strength
      Hero hero(HeroClass::Transmuter, HeroRace::Elf);
      AssertThat(hero.getManaPoints(), Equals(10));
      hero.addConversionPoints(70, noOtherMonsters);
      AssertThat(hero.getManaPointsMax(), Equals(11));
      AssertThat(hero.getManaPoints(), Equals(0));
      AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength),
                 Equals(hero.getLevel() + hero.getManaPointsMax()));
    });
  });
}
