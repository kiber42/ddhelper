#include "bandit/bandit.h"

#include "engine/Monster.hpp"
#include "engine/MonsterStats.hpp"

#include <array>
#include <random>

using namespace bandit;
using namespace snowhouse;

void testMonsterBasics()
{
  describe("Monster", [] {
    Monster monster(2, 10, 3);
    it("used for test should have level 2 and 10 HP", [&] {
      AssertThat(monster.getLevel(), Equals(2u));
      AssertThat(monster.getHitPoints(), Equals(10u));
    });
    it("with 10 HP should survive a hit with 9 damage points and has 1 HP remaining", [&] {
      monster.takeDamage(9, DamageType::Physical);
      AssertThat(monster.isDefeated(), IsFalse());
      AssertThat(monster.getHitPoints(), Equals(1u));
    });
    it("at level 2 should recover at a rate of 2 HP per explored square", [&] {
      monster.recover(4);
      AssertThat(monster.getHitPoints(), Equals(9u));
    });
    it("should not recover beyond its max HP", [&] {
      monster.recover(10);
      AssertThat(monster.getHitPoints(), Equals(monster.getHitPointsMax()));
    });
    it("should not recover HP while poisoned", [&] {
      monster.takeDamage(1, DamageType::Physical);
      monster.poison(3);
      monster.recover(1);
      AssertThat(monster.getHitPoints(), Equals(9u));
      AssertThat(monster.getPoisonAmount(), Equals(1u));
    });
    it("should reduce poison as it would usually recover HP", [&] {
      monster.recover(1);
      AssertThat(monster.getHitPoints(), Equals(10u));
      AssertThat(monster.isPoisoned(), IsFalse());
    });
    it("should lose 4 HP per caster level when hit by a fireball", [&] {
      monster.takeFireballDamage(2);
      AssertThat(monster.getHitPoints(), Equals(10u - 2u * 4u));
    });
    it("should be burning after hit by a fireball", [&] {
      AssertThat(monster.isBurning(), IsTrue());
      AssertThat(monster.getBurnStackSize(), Equals(1u));
    });
    it("should recover HP at a rate reduced by 1 when burning", [&] {
      monster.recover(4);
      AssertThat(monster.getHitPoints(), Equals(6u));
    });
    it("should take additional fireball damage when already burning", [&] {
      monster.takeFireballDamage(1);
      AssertThat(monster.getHitPoints(), Equals(6u - 1u * 4u - 1u));
      AssertThat(monster.getBurnStackSize(), Equals(2u));
    });
    it("should recover HP at a rate reduced by 1 when burning, independent of burn stack size", [&] {
      monster.recover(5);
      AssertThat(monster.getHitPoints(), Equals(6u));
    });
    it("should take additional fireball damage per burn stack", [&] {
      monster.recover(10);
      monster.takeFireballDamage(1);
      AssertThat(monster.getHitPoints(), Equals(10u - 1u * 4u - 2u));
    });
    it("should not have a burn stack size higher than twice the caster's level",
       [&] { AssertThat(monster.getBurnStackSize(), Equals(2u)); });
    it("should stop burning upon any physical damage, and take damage equal to burn stack size", [&] {
      AssertThat(monster.getHitPoints() - monster.getBurnStackSize(), Equals(2u));
      monster.takeDamage(0, DamageType::Physical);
      AssertThat(monster.isBurning(), IsFalse());
      AssertThat(monster.getHitPoints(), Equals(2u));
    });
    it("should recover from being slowed when taking damage", [&] {
      monster.slow();
      AssertThat(monster.isSlowed(), IsTrue());
      monster.takeDamage(1, DamageType::Physical);
      AssertThat(monster.isSlowed(), IsFalse());
    });
  });
}

void testHiddenMonster()
{
  describe("Hidden Monster", [] {
    std::mt19937 generator{0};
    Monster monster(MonsterType::BloodSnake, Level{7}, DungeonMultiplier{2});
    HiddenMonster hiddenSpecific(std::move(monster));
    it("should allow to set and reveal a specific monster", [&] {
      auto revealedMonster = hiddenSpecific.reveal(generator);
      AssertThat(revealedMonster.getLevel(), Equals(7u));
      AssertThat(revealedMonster.getHitPoints(), Equals(180u));
      AssertThat(revealedMonster.getName(), Equals("Blood Snake level 7"));
    });
    it("should reveal random monsters of same level if called repeatedly", [&] {
      auto revealedMonster = hiddenSpecific.reveal(generator);
      revealedMonster = hiddenSpecific.reveal(generator);
      // Dungeon multiplier is reset to 100%
      AssertThat(revealedMonster.getHitPoints(), IsLessThan(100u));
      AssertThat(revealedMonster.getName(), !Equals("Blood Snake level 7"));
    });
    HiddenMonster hiddenBasic(Level{5}, DungeonMultiplier{0.1f}, false);
    it("should allow to set and reveal a random basic monster of given level", [&] {
      for (int count = 0; count < 10; ++count)
      {
        auto revealedMonster = hiddenBasic.reveal(generator);
        AssertThat(revealedMonster.getLevel(), Equals(5u));
        AssertThat(revealedMonster.getHitPoints(), IsLessThan(20u));
        // Have to verify that a basic monster was revealed using its name
        std::array<std::string, (int)MonsterType::LastBasic + 1> basic_names;
        using namespace std::string_literals;
        for (auto i = 0u; i <= (int)MonsterType::LastBasic; ++i)
          basic_names[i] = toString((MonsterType)i) + " level 5"s;
        AssertThat(basic_names, Contains(revealedMonster.getName()));
      }
    });
  });
}

void testMonsterHitPoints()
{
  describe("Monster hitpoints", [] {
    it("should match those of real game (normal difficulty)", [] {
      AssertThat(Monster(MonsterType::Bandit, Level{1}).getHitPoints(), Equals(6u));
      AssertThat(Monster(MonsterType::Bandit, Level{2}).getHitPoints(), Equals(15u));
      AssertThat(Monster(MonsterType::Bandit, Level{4}).getHitPoints(), Equals(39u));
      AssertThat(Monster(MonsterType::Bandit, Level{7}).getHitPoints(), Equals(90u));
      AssertThat(Monster(MonsterType::DesertTroll, Level{1}).getHitPoints(), Equals(8u));
      AssertThat(Monster(MonsterType::DesertTroll, Level{2}).getHitPoints(), Equals(20u));
      AssertThat(Monster(MonsterType::DesertTroll, Level{3}).getHitPoints(), Equals(36u));
      AssertThat(Monster(MonsterType::DesertTroll, Level{4}).getHitPoints(), Equals(54u));
      AssertThat(Monster(MonsterType::DesertTroll, Level{5}).getHitPoints(), Equals(75u));
      AssertThat(Monster(MonsterType::DesertTroll, Level{7}).getHitPoints(), Equals(125u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{1}).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Goat, Level{1}).getHitPoints(), Equals(5u));
      AssertThat(Monster(MonsterType::Goat, Level{2}).getHitPoints(), Equals(13u));
      AssertThat(Monster(MonsterType::Goat, Level{3}).getHitPoints(), Equals(23u));
      AssertThat(Monster(MonsterType::Goat, Level{8}).getHitPoints(), Equals(99u));
      AssertThat(Monster(MonsterType::Goblin, Level{1}).getHitPoints(), Equals(6u));
      AssertThat(Monster(MonsterType::Goblin, Level{3}).getHitPoints(), Equals(26u));
      AssertThat(Monster(MonsterType::Goblin, Level{6}).getHitPoints(), Equals(71u));
      AssertThat(Monster(MonsterType::Goblin, Level{8}).getHitPoints(), Equals(111u));
      AssertThat(Monster(MonsterType::Goblin, Level{9}).getHitPoints(), Equals(134u));
      AssertThat(Monster(MonsterType::Golem, Level{2}).getHitPoints(), Equals(15u));
      AssertThat(Monster(MonsterType::Golem, Level{3}).getHitPoints(), Equals(26u));
      AssertThat(Monster(MonsterType::Golem, Level{7}).getHitPoints(), Equals(90u));
      AssertThat(Monster(MonsterType::Golem, Level{8}).getHitPoints(), Equals(111u));
      AssertThat(Monster(MonsterType::GooBlob, Level{1}).getHitPoints(), Equals(6u));
      AssertThat(Monster(MonsterType::GooBlob, Level{2}).getHitPoints(), Equals(15u));
      AssertThat(Monster(MonsterType::GooBlob, Level{5}).getHitPoints(), Equals(54u));
      AssertThat(Monster(MonsterType::GooBlob, Level{6}).getHitPoints(), Equals(71u));
      AssertThat(Monster(MonsterType::GooBlob, Level{7}).getHitPoints(), Equals(90u));
      AssertThat(Monster(MonsterType::Gorgon, Level{1}).getHitPoints(), Equals(5u));
      AssertThat(Monster(MonsterType::Gorgon, Level{3}).getHitPoints(), Equals(23u));
      AssertThat(Monster(MonsterType::Gorgon, Level{4}).getHitPoints(), Equals(35u));
      AssertThat(Monster(MonsterType::Gorgon, Level{5}).getHitPoints(), Equals(48u));
      AssertThat(Monster(MonsterType::MeatMan, Level{1}).getHitPoints(), Equals(12u));
      AssertThat(Monster(MonsterType::MeatMan, Level{2}).getHitPoints(), Equals(30u));
      AssertThat(Monster(MonsterType::MeatMan, Level{5}).getHitPoints(), Equals(108u));
      AssertThat(Monster(MonsterType::MeatMan, Level{8}).getHitPoints(), Equals(222u));
      AssertThat(Monster(MonsterType::MeatMan, Level{9}).getHitPoints(), Equals(268u));
      AssertThat(Monster(MonsterType::Serpent, Level{1}).getHitPoints(), Equals(6u));
      AssertThat(Monster(MonsterType::Serpent, Level{2}).getHitPoints(), Equals(15u));
      AssertThat(Monster(MonsterType::Serpent, Level{5}).getHitPoints(), Equals(54u));
      AssertThat(Monster(MonsterType::Serpent, Level{6}).getHitPoints(), Equals(71u));
      AssertThat(Monster(MonsterType::Warlock, Level{3}).getHitPoints(), Equals(26u));
      AssertThat(Monster(MonsterType::Warlock, Level{4}).getHitPoints(), Equals(39u));
      AssertThat(Monster(MonsterType::Warlock, Level{7}).getHitPoints(), Equals(90u));
      AssertThat(Monster(MonsterType::Warlock, Level{8}).getHitPoints(), Equals(111u));
      AssertThat(Monster(MonsterType::Warlock, Level{10}).getHitPoints(), Equals(159u));
      AssertThat(Monster(MonsterType::Wraith, Level{1}).getHitPoints(), Equals(4u));
      AssertThat(Monster(MonsterType::Wraith, Level{2}).getHitPoints(), Equals(11u));
      AssertThat(Monster(MonsterType::Wraith, Level{3}).getHitPoints(), Equals(19u));
      AssertThat(Monster(MonsterType::Wraith, Level{4}).getHitPoints(), Equals(29u));
      AssertThat(Monster(MonsterType::Wraith, Level{5}).getHitPoints(), Equals(40u));
      AssertThat(Monster(MonsterType::Wraith, Level{6}).getHitPoints(), Equals(53u));
      AssertThat(Monster(MonsterType::Wraith, Level{9}).getHitPoints(), Equals(100u));
      AssertThat(Monster(MonsterType::Zombie, Level{1}).getHitPoints(), Equals(9u));
      AssertThat(Monster(MonsterType::Zombie, Level{4}).getHitPoints(), Equals(58u));
      AssertThat(Monster(MonsterType::Zombie, Level{6}).getHitPoints(), Equals(106u));
      AssertThat(Monster(MonsterType::Zombie, Level{8}).getHitPoints(), Equals(166u));
      AssertThat(Monster(MonsterType::Zombie, Level{9}).getHitPoints(), Equals(201u));
    });
    it("should match those of real game (130% difficulty)", [] {
      // rounding matters and appears to be different between C# and C++
      const auto dm = DungeonMultiplier{1.299f};
      AssertThat(Monster(MonsterType::Bandit, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Bandit, Level{4}, dm).getHitPoints(), Equals(50u));
      AssertThat(Monster(MonsterType::Bandit, Level{7}, dm).getHitPoints(), Equals(116u));
      AssertThat(Monster(MonsterType::Changeling, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Changeling, Level{2}, dm).getHitPoints(), Equals(19u));
      AssertThat(Monster(MonsterType::Changeling, Level{6}, dm).getHitPoints(), Equals(92u));
      AssertThat(Monster(MonsterType::Changeling, Level{7}, dm).getHitPoints(), Equals(116u));
      AssertThat(Monster(MonsterType::Djinn, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Djinn, Level{2}, dm).getHitPoints(), Equals(20u));
      AssertThat(Monster(MonsterType::Djinn, Level{3}, dm).getHitPoints(), Equals(36u));
      AssertThat(Monster(MonsterType::Djinn, Level{6}, dm).getHitPoints(), Equals(101u));
      AssertThat(Monster(MonsterType::Djinn, Level{7}, dm).getHitPoints(), Equals(127u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{1}, dm).getHitPoints(), Equals(8u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{2}, dm).getHitPoints(), Equals(23u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{3}, dm).getHitPoints(), Equals(41u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{4}, dm).getHitPoints(), Equals(62u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{5}, dm).getHitPoints(), Equals(87u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{7}, dm).getHitPoints(), Equals(145u));
      AssertThat(Monster(MonsterType::DragonSpawn, Level{9}, dm).getHitPoints(), Equals(217u));
      AssertThat(Monster(MonsterType::Goat, Level{1}, dm).getHitPoints(), Equals(6u));
      AssertThat(Monster(MonsterType::Goat, Level{2}, dm).getHitPoints(), Equals(17u));
      AssertThat(Monster(MonsterType::Goat, Level{3}, dm).getHitPoints(), Equals(29u));
      AssertThat(Monster(MonsterType::Goat, Level{4}, dm).getHitPoints(), Equals(44u));
      AssertThat(Monster(MonsterType::Goat, Level{6}, dm).getHitPoints(), Equals(82u));
      AssertThat(Monster(MonsterType::Goat, Level{7}, dm).getHitPoints(), Equals(104u));
      AssertThat(Monster(MonsterType::Goat, Level{8}, dm).getHitPoints(), Equals(129u));
      AssertThat(Monster(MonsterType::Goat, Level{9}, dm).getHitPoints(), Equals(156u));
      AssertThat(Monster(MonsterType::Goblin, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Goblin, Level{3}, dm).getHitPoints(), Equals(33u));
      AssertThat(Monster(MonsterType::Goblin, Level{6}, dm).getHitPoints(), Equals(92u));
      AssertThat(Monster(MonsterType::Goblin, Level{7}, dm).getHitPoints(), Equals(116u));
      AssertThat(Monster(MonsterType::Goblin, Level{8}, dm).getHitPoints(), Equals(144u));
      AssertThat(Monster(MonsterType::Golem, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Golem, Level{2}, dm).getHitPoints(), Equals(19u));
      AssertThat(Monster(MonsterType::Golem, Level{3}, dm).getHitPoints(), Equals(33u));
      AssertThat(Monster(MonsterType::Golem, Level{7}, dm).getHitPoints(), Equals(116u));
      AssertThat(Monster(MonsterType::Golem, Level{8}, dm).getHitPoints(), Equals(144u));
      AssertThat(Monster(MonsterType::GooBlob, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::GooBlob, Level{3}, dm).getHitPoints(), Equals(33u));
      AssertThat(Monster(MonsterType::GooBlob, Level{5}, dm).getHitPoints(), Equals(70u));
      AssertThat(Monster(MonsterType::GooBlob, Level{6}, dm).getHitPoints(), Equals(92u));
      AssertThat(Monster(MonsterType::Gorgon, Level{1}, dm).getHitPoints(), Equals(6u));
      AssertThat(Monster(MonsterType::Gorgon, Level{3}, dm).getHitPoints(), Equals(29u));
      AssertThat(Monster(MonsterType::Gorgon, Level{6}, dm).getHitPoints(), Equals(82u));
      AssertThat(Monster(MonsterType::Gorgon, Level{9}, dm).getHitPoints(), Equals(156u));
      AssertThat(Monster(MonsterType::MeatMan, Level{1}, dm).getHitPoints(), Equals(14u));
      AssertThat(Monster(MonsterType::MeatMan, Level{2}, dm).getHitPoints(), Equals(38u));
      AssertThat(Monster(MonsterType::MeatMan, Level{3}, dm).getHitPoints(), Equals(66u));
      AssertThat(Monster(MonsterType::MeatMan, Level{4}, dm).getHitPoints(), Equals(100u));
      AssertThat(Monster(MonsterType::MeatMan, Level{5}, dm).getHitPoints(), Equals(140u));
      AssertThat(Monster(MonsterType::MeatMan, Level{6}, dm).getHitPoints(), Equals(184u));
      AssertThat(Monster(MonsterType::MeatMan, Level{8}, dm).getHitPoints(), Equals(288u));
      AssertThat(Monster(MonsterType::Minotaur, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Minotaur, Level{2}, dm).getHitPoints(), Equals(19u));
      AssertThat(Monster(MonsterType::Minotaur, Level{3}, dm).getHitPoints(), Equals(33u));
      AssertThat(Monster(MonsterType::Minotaur, Level{5}, dm).getHitPoints(), Equals(70u));
      AssertThat(Monster(MonsterType::Minotaur, Level{8}, dm).getHitPoints(), Equals(144u));
      AssertThat(Monster(MonsterType::Minotaur, Level{9}, dm).getHitPoints(), Equals(174u));
      AssertThat(Monster(MonsterType::Naga, Level{1}, dm).getHitPoints(), Equals(5u));
      AssertThat(Monster(MonsterType::Naga, Level{5}, dm).getHitPoints(), Equals(59u));
      AssertThat(Monster(MonsterType::Naga, Level{6}, dm).getHitPoints(), Equals(78u));
      AssertThat(Monster(MonsterType::Naga, Level{7}, dm).getHitPoints(), Equals(98u));
      AssertThat(Monster(MonsterType::Naga, Level{8}, dm).getHitPoints(), Equals(122u));
      AssertThat(Monster(MonsterType::Naga, Level{9}, dm).getHitPoints(), Equals(147u));
      AssertThat(Monster(MonsterType::Serpent, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Serpent, Level{2}, dm).getHitPoints(), Equals(19u));
      AssertThat(Monster(MonsterType::Serpent, Level{4}, dm).getHitPoints(), Equals(50u));
      AssertThat(Monster(MonsterType::Serpent, Level{5}, dm).getHitPoints(), Equals(70u));
      AssertThat(Monster(MonsterType::Serpent, Level{7}, dm).getHitPoints(), Equals(116u));
      AssertThat(Monster(MonsterType::Warlock, Level{1}, dm).getHitPoints(), Equals(7u));
      AssertThat(Monster(MonsterType::Warlock, Level{2}, dm).getHitPoints(), Equals(19u));
      AssertThat(Monster(MonsterType::Warlock, Level{5}, dm).getHitPoints(), Equals(70u));
      AssertThat(Monster(MonsterType::Warlock, Level{8}, dm).getHitPoints(), Equals(144u));
      AssertThat(Monster(MonsterType::Wraith, Level{1}, dm).getHitPoints(), Equals(5u));
      AssertThat(Monster(MonsterType::Wraith, Level{2}, dm).getHitPoints(), Equals(14u));
      AssertThat(Monster(MonsterType::Wraith, Level{3}, dm).getHitPoints(), Equals(24u));
      AssertThat(Monster(MonsterType::Wraith, Level{4}, dm).getHitPoints(), Equals(37u));
      AssertThat(Monster(MonsterType::Wraith, Level{5}, dm).getHitPoints(), Equals(52u));
      AssertThat(Monster(MonsterType::Wraith, Level{7}, dm).getHitPoints(), Equals(87u));
      AssertThat(Monster(MonsterType::Wraith, Level{8}, dm).getHitPoints(), Equals(108u));
      AssertThat(Monster(MonsterType::Zombie, Level{1}, dm).getHitPoints(), Equals(10u));
      AssertThat(Monster(MonsterType::Zombie, Level{2}, dm).getHitPoints(), Equals(28u));
      AssertThat(Monster(MonsterType::Zombie, Level{3}, dm).getHitPoints(), Equals(49u));
      AssertThat(Monster(MonsterType::Zombie, Level{4}, dm).getHitPoints(), Equals(75u));
      AssertThat(Monster(MonsterType::Zombie, Level{5}, dm).getHitPoints(), Equals(105u));
      AssertThat(Monster(MonsterType::Zombie, Level{6}, dm).getHitPoints(), Equals(138u));
      AssertThat(Monster(MonsterType::Zombie, Level{8}, dm).getHitPoints(), Equals(216u));
    });
  });
}

void testMonsters()
{
  testMonsterBasics();
  testMonsterHitPoints();
  testHiddenMonster();
}
