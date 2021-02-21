#include "bandit/bandit.h"

#include "Dungeon.hpp"
#include "Hero.hpp"
#include "Monster.hpp"

using namespace bandit;
using namespace snowhouse;

void testDungeonBasics()
{
  describe("Dungeon", [] {
    Dungeon dungeon(3, 3);
    auto monster = std::make_shared<Monster>(3, 30, 10);
    auto monster2 = std::make_shared<Monster>(*monster);
    it("should allow adding monsters if there is space", [&] {
      dungeon.add(monster, dungeon.randomFreePosition().value());
      AssertThat(dungeon.getMonsters().size(), Equals(1u));
      for (unsigned n = 2; n <= 9; ++n)
      {
        AssertThat(dungeon.randomFreePosition().has_value(), IsTrue());
        dungeon.add(monster2, dungeon.randomFreePosition().value());
        AssertThat(dungeon.getMonsters().size(), Equals(n));
      }
      AssertThat(dungeon.randomFreePosition().has_value(), IsFalse());
      AssertThat(dungeon.isFree({0, 0}), IsFalse());
    });
    it("should not be revealed initially", [&] { AssertThat(dungeon.isRevealed({1, 1}), IsFalse()); });
    it("should consider a square with a defeated monster as free", [&] {
      monster->takeDamage(100, false);
      AssertThat(monster->isDefeated(), IsTrue());
      dungeon.update();
      AssertThat(dungeon.randomFreePosition().has_value(), IsTrue());
    });
    auto hero = std::make_shared<Hero>();
    it("should consider the hero's square as occupied", [&] {
      dungeon.setHero(hero, dungeon.randomFreePosition().value());
      AssertThat(dungeon.randomFreePosition().has_value(), IsFalse());
    });
    it("should be revealed around the hero's position", [&] { AssertThat(dungeon.isRevealed({1, 1}), IsTrue()); });
  });

  describe("Pathfinding", [] {
    Dungeon dungeon(10, 10);
    auto hero = std::make_shared<Hero>();
    dungeon.setHero(hero, Position(0, 0));
    it("should find paths if there are no obstacles", [&] { AssertThat(dungeon.isConnected({9, 9}), IsTrue()); });
    it("should consider squares inaccessible if there are no revealed paths to them", [&] {
      AssertThat(dungeon.isAccessible({9, 9}), IsFalse());
    });
    it("should consider squares accessible if there is a revealed path", [&] {
      for (int x = 2; x < 10; ++x)
        dungeon.reveal({x, x});
      AssertThat(dungeon.isAccessible({9, 9}), IsTrue());
    });
  });
}

void testDungeon()
{
  testDungeonBasics();
}
