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
      hero.use(Item::HealthPotion, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(expected));
    });
    it("removes poison", [] {
      Hero hero;
      hero.addStatus(HeroDebuff::Poisoned, noOtherMonsters);
      hero.use(Item::HealthPotion, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroDebuff::Poisoned), IsFalse());
    });
    it("does not overheal", [] {
      Hero hero;
      hero.healHitPoints(1, true);
      AssertThat(hero.getHitPoints(), Equals(11));
      hero.use(Item::HealthPotion, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(11));
    });
  });
  describe("Mana Potion", [] {
    it("restores 40% of max MP", [] {
      Hero hero;
      hero.setManaPointsMax(99);
      const int expected = hero.getManaPoints() + 39;
      hero.use(Item::ManaPotion, noOtherMonsters);
      AssertThat(hero.getManaPoints(), Equals(expected));
      hero.use(Item::ManaPotion, noOtherMonsters);
      hero.use(Item::ManaPotion, noOtherMonsters);
      AssertThat(hero.getManaPoints(), Equals(99));
    });
    it("removes mana burn", [] {
      Hero hero;
      hero.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
      hero.use(Item::ManaPotion, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroDebuff::ManaBurned), IsFalse());
    });
  });
  describe("Fortitude Tonic", [] {
    it("removes poison and weakening", [] {
      Hero hero;
      hero.addStatus(HeroDebuff::Poisoned, noOtherMonsters);
      hero.addStatus(HeroDebuff::Weakened, noOtherMonsters, 50);
      hero.use(Item::FortitudeTonic, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroDebuff::Poisoned), IsFalse());
      AssertThat(hero.hasStatus(HeroDebuff::Weakened), IsFalse());
    });
  });
  describe("Burn Salve", [] {
    it("removes mana burn and corrosion", [] {
      Hero hero;
      hero.addStatus(HeroDebuff::ManaBurned, noOtherMonsters);
      hero.addStatus(HeroDebuff::Corroded, noOtherMonsters, 50);
      hero.use(Item::BurnSalve, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroDebuff::ManaBurned), IsFalse());
      AssertThat(hero.hasStatus(HeroDebuff::Corroded), IsFalse());
    });
  });
  describe("Strength Potion", [] {
    it("Converts all mana into spirit strength", [] {
      Hero hero;
      hero.gainLevel(noOtherMonsters);
      hero.setManaPointsMax(123);
      hero.refillHealthAndMana();
      hero.use(Item::StrengthPotion, noOtherMonsters);
      AssertThat(hero.getManaPoints(), Equals(0));
      AssertThat(hero.getStatusIntensity(HeroStatus::SpiritStrength), Equals(123 + hero.getLevel()));
      AssertThat(hero.getDamageVersusStandard(), Equals(135));
    });
  });
  describe("Schadenfreude", [] {
    it("adds Schadenfreude status", [] {
      Hero hero;
      hero.use(Item::Schadenfreude, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroStatus::Schadenfreude), IsTrue());
    });
  });
  describe("Quicksilver Potion", [] {
    it("adds 50% dodge change (temporary)", [] {
      Hero hero;
      hero.use(Item::QuicksilverPotion, noOtherMonsters);
      AssertThat(hero.getDodgeChancePercent(), Equals(50));
      AssertThat(hero.hasStatus(HeroStatus::DodgePermanent), Equals(0));
    });
    it("adds dodge prediction", [] {
      Hero hero;
      hero.use(Item::QuicksilverPotion, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroStatus::DodgePrediction), IsTrue());
    });
  });
  describe("Reflex Potion", [] {
    it("adds Reflexes status", [] {
      Hero hero;
      hero.use(Item::ReflexPotion, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroStatus::Reflexes), IsTrue());
    });
  });
  describe("Can Of Whupaz", [] {
    it("adds Crushing Blow status", [] {
      Hero hero;
      hero.use(Item::CanOfWhupaz, noOtherMonsters);
      AssertThat(hero.hasStatus(HeroStatus::CrushingBlow), IsTrue());
    });
  });
}
