#include "bandit/bandit.h"

#include "engine/Faith.hpp"
#include "engine/Hero.hpp"
#include "engine/Items.hpp"

using namespace bandit;
using namespace snowhouse;

namespace
{
  Monsters noOtherMonsters;
} // namespace

void testItems()
{
  describe("Pendant of Health", [] {
    it("shall increase max health by 10", [] {
      Hero hero;
      hero.takeDamage(2, DamageType::Physical, noOtherMonsters);
      AssertThat(hero.receive(ShopItem::PendantOfHealth), IsTrue());
      AssertThat(hero.getHitPoints(), Equals(8u));
      AssertThat(hero.getHitPointsMax(), Equals(20u));
      hero.recover(100, noOtherMonsters);
      AssertThat(hero.getHitPoints(), Equals(20u));
      AssertThat(hero.getHitPointsMax(), Equals(20u));
    });
    it("shall decrease max health by 10 when converted", [] {
      Hero priest(HeroClass::Priest, HeroRace::Dwarf);
      AssertThat(priest.getHitPoints(), Equals(13u));
      AssertThat(priest.getHitPointsMax(), Equals(13u));
      priest.addConversionPoints(100, noOtherMonsters);
      AssertThat(priest.getHitPoints(), Equals(13u));
      AssertThat(priest.getHitPointsMax(), Equals(14u));
      AssertThat(priest.receive(ShopItem::PendantOfHealth), IsTrue());
      AssertThat(priest.getHitPoints(), Equals(13u));
      AssertThat(priest.getHitPointsMax(), Equals(24u));
      priest.convert(ShopItem::PendantOfHealth, noOtherMonsters);
      AssertThat(priest.getHitPoints(), Equals(13u));
      AssertThat(priest.getHitPointsMax(), Equals(14u));
    });
    it("shall respect max. HP when converted", [] {
      Hero assassin(HeroClass::Assassin, HeroRace::Elf);
      AssertThat(assassin.receive(ShopItem::PendantOfHealth), IsTrue());
      assassin.recover(10, noOtherMonsters);
      AssertThat(assassin.getHitPoints(), Equals(20u));
      AssertThat(assassin.getHitPointsMax(), Equals(20u));
      assassin.convert(ShopItem::PendantOfHealth, noOtherMonsters);
      AssertThat(assassin.getHitPoints(), Equals(10u));
      AssertThat(assassin.getHitPointsMax(), Equals(10u));
    });
    it("shall respect overhealing when converted", [] {
      Hero vampire(HeroClass::Vampire);
      AssertThat(vampire.receive(ShopItem::PendantOfHealth), IsTrue());
      AssertThat(vampire.receive(ShopItem::PendantOfHealth), IsTrue());
      vampire.gainLevel(noOtherMonsters);
      vampire.healHitPoints(1, true);
      AssertThat(vampire.getHitPoints(), Equals(41u));
      AssertThat(vampire.getHitPointsMax(), Equals(40u));
      vampire.convert(ShopItem::PendantOfHealth, noOtherMonsters);
      AssertThat(vampire.getHitPoints(), Equals(41u));
      AssertThat(vampire.getHitPointsMax(), Equals(30u));
      vampire.convert(ShopItem::PendantOfHealth, noOtherMonsters);
      AssertThat(vampire.getHitPoints(), Equals(30u));
      AssertThat(vampire.getHitPointsMax(), Equals(20u));
    });
    it("shall not decrease max health below 1 when converted", [] {
      Hero hero;
      AssertThat(hero.receive(ShopItem::PendantOfHealth), IsTrue());
      AssertThat(hero.desecrate(God::Dracul, noOtherMonsters), IsTrue());
      AssertThat(hero.isDefeated(), IsFalse());
      AssertThat(hero.getHitPoints(), Equals(1u));
      AssertThat(hero.getHitPointsMax(), Equals(1u));
      hero.convert(ShopItem::PendantOfHealth, noOtherMonsters);
      AssertThat(hero.isDefeated(), IsFalse());
      AssertThat(hero.getHitPoints(), Equals(1u));
      AssertThat(hero.getHitPointsMax(), Equals(1u));
    });
  });
}
