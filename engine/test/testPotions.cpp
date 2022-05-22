#include "bandit/bandit.h"

#include "engine/Hero.hpp"
#include "engine/Items.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
}

void testPotions()
{
  describe("Health Potion", [] {
    it("restores 40% of max HP", [] {
      Hero hero;
      hero.setHitPointsMax(99);
      const int expected = hero.getHitPoints() + 39;
      hero.use(Potion::HealthPotion, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(expected));
    });
    it("removes poison", [] {
      Hero hero;
      hero.add(HeroDebuff::Poisoned, noOtherMonsters);
      hero.use(Potion::HealthPotion, noOtherMonsters);
      AssertThat(hero.has(HeroDebuff::Poisoned), IsFalse());
    });
    it("does not overheal", [] {
      Hero hero;
      hero.healHitPoints(1, true);
      AssertThat(hero.getHitPoints(), Equals(11));
      hero.use(Potion::HealthPotion, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(11));
    });
  });
  describe("Mana Potion", [] {
    it("restores 40% of max MP", [] {
      Hero hero;
      hero.setManaPointsMax(99);
      const int expected = hero.getManaPoints() + 39;
      hero.use(Potion::ManaPotion, noOtherMonsters);
      AssertThat(hero.getManaPoints(), Equals(expected));
      hero.use(Potion::ManaPotion, noOtherMonsters);
      hero.use(Potion::ManaPotion, noOtherMonsters);
      AssertThat(hero.getManaPoints(), Equals(99));
    });
    it("removes mana burn", [] {
      Hero hero;
      hero.add(HeroDebuff::ManaBurned, noOtherMonsters);
      hero.use(Potion::ManaPotion, noOtherMonsters);
      AssertThat(hero.has(HeroDebuff::ManaBurned), IsFalse());
    });
  });
  describe("Fortitude Tonic", [] {
    it("removes poison and weakening", [] {
      Hero hero;
      hero.add(HeroDebuff::Poisoned, noOtherMonsters);
      hero.add(HeroDebuff::Weakened, noOtherMonsters, 50);
      hero.use(Potion::FortitudeTonic, noOtherMonsters);
      AssertThat(hero.has(HeroDebuff::Poisoned), IsFalse());
      AssertThat(hero.has(HeroDebuff::Weakened), IsFalse());
    });
  });
  describe("Burn Salve", [] {
    it("removes mana burn and corrosion", [] {
      Hero hero;
      hero.add(HeroDebuff::ManaBurned, noOtherMonsters);
      hero.add(HeroDebuff::Corroded, noOtherMonsters, 50);
      hero.use(Potion::BurnSalve, noOtherMonsters);
      AssertThat(hero.has(HeroDebuff::ManaBurned), IsFalse());
      AssertThat(hero.has(HeroDebuff::Corroded), IsFalse());
    });
  });
  describe("Strength Potion", [] {
    it("Converts all mana into spirit strength", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      hero.setManaPointsMax(123);
      hero.refillHealthAndMana();
      hero.use(Potion::StrengthPotion, noOtherMonsters);
      AssertThat(hero.getManaPoints(), Equals(0));
      AssertThat(hero.getIntensity(HeroStatus::SpiritStrength), Equals(123 + hero.getLevel()));
      AssertThat(hero.getDamageVersusStandard(), Equals(135));
    });
  });
  describe("Schadenfreude", [] {
    it("adds Schadenfreude status", [] {
      Hero hero;
      hero.use(Potion::Schadenfreude, noOtherMonsters);
      AssertThat(hero.has(HeroStatus::Schadenfreude), IsTrue());
    });
  });
  describe("Quicksilver Potion", [] {
    it("adds 50% dodge change (temporary)", [] {
      Hero hero;
      hero.use(Potion::QuicksilverPotion, noOtherMonsters);
      AssertThat(hero.getDodgeChancePercent(), Equals(50u));
      AssertThat(hero.has(HeroStatus::DodgePermanent), IsFalse());
    });
    it("adds dodge prediction", [] {
      Hero hero;
      hero.use(Potion::QuicksilverPotion, noOtherMonsters);
      AssertThat(hero.has(HeroStatus::DodgePrediction), IsTrue());
    });
  });
  describe("Reflex Potion", [] {
    it("adds Reflexes status", [] {
      Hero hero;
      hero.use(Potion::ReflexPotion, noOtherMonsters);
      AssertThat(hero.has(HeroStatus::Reflexes), IsTrue());
    });
  });
  describe("Can Of Whupaz", [] {
    it("adds Crushing Blow status", [] {
      Hero hero;
      hero.use(Potion::CanOfWhupaz, noOtherMonsters);
      AssertThat(hero.has(HeroStatus::CrushingBlow), IsTrue());
    });
  });
}
