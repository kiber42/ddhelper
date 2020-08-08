#include "cute.h"
#include "cute_runner.h"
#include "ide_listener.h"
#include "xml_listener.h"

#include "Dungeon.hpp"
#include "Experience.hpp"
#include "Hero.hpp"
#include "Melee.hpp"
#include "Monster.hpp"
#include "MonsterFactory.hpp"

void testHeroExperienceBasic()
{
  Hero hero;
  ASSERT_EQUALM("A hero is created with level 1", 1, hero.getLevel());
  hero.gainExperience(5);
  ASSERT_EQUALM("After receiving 5 XP the hero is level 2", 2, hero.getLevel());
  hero.gainLevel();
  ASSERT_EQUALM("Level gain triggered", 3, hero.getLevel());
  hero.gainExperience(1000);
  ASSERT_EQUALM("Hero level cannot exceed 10", 10, hero.getLevel());
  ASSERT_EQUALM("Hero receives prestige instead", 10, hero.getPrestige());
  hero.modifyLevelBy(-1);
  ASSERT_EQUALM("A hero's level can be artificially reduced (boon)", 9, hero.getLevel());
  ASSERT_EQUALM("Number of hero's prestige unaffected by boon", 10, hero.getPrestige());
  hero.modifyLevelBy(-10);
  ASSERT_EQUALM("A hero's level cannot be artificially reduced below 1", 1, hero.getLevel());
  hero.modifyLevelBy(+10);
  ASSERT_EQUALM("A hero's level cannot be artificially increased above 10", 10, hero.getLevel());
}

void testHeroExperienceAdvanced()
{
  {
    Hero hero;
    hero.loseHitPointsOutsideOfFight(4);
    hero.gainExperience(5);
    ASSERT_EQUALM("Hit points refilled on level up (from XP)", hero.getHitPointsMax(), hero.getHitPoints());
    ASSERT_EQUALM("Maximum hit points raised on level up (from XP)", 20, hero.getHitPointsMax());
    hero.loseHitPointsOutsideOfFight(1);
    hero.gainLevel();
    ASSERT_EQUALM("Hit points raised and refilled on direct level up", 30, hero.getHitPoints());
  }
  {
    Hero hero;
    hero.addStatus(HeroStatus::Learning);
    hero.gainExperience(1);
    ASSERT_EQUALM("After receiving 1 XP a fresh hero with 'Learning' has 2 XP", 2, hero.getXP());
    ASSERT_EQUALM("The 'Learning' status persists", 1, hero.hasStatus(HeroStatus::Learning));
    hero.addStatus(HeroStatus::Learning);
    ASSERT_EQUALM("Another level of 'Learning' can be added", 2, hero.hasStatus(HeroStatus::Learning));
    hero.gainExperience(1);
    ASSERT_EQUALM("After receiving another 1 XP the hero has leveled up (5 XP)", 2, hero.getLevel());
  }
  {
    Hero hero;
    hero.addStatus(HeroStatus::ExperienceBoost);
    hero.gainExperience(2);
    ASSERT_EQUALM("After receiving 2 XP a fresh hero with 'Experience Boost' has 3 XP", 3, hero.getXP());
    hero.gainExperience(2);
    ASSERT_EQUALM("The 'Experience Boost' status is automatically removed", 0,
                  hero.hasStatus(HeroStatus::ExperienceBoost));
    hero.gainLevel();
    hero.addStatus(HeroStatus::ExperienceBoost);
    hero.addStatus(HeroStatus::Learning);
    hero.gainExperience(3); /* 150/100 * 3 + 1 rather than 150/100 * (3 + 1) */
    ASSERT_EQUALM("Learning is applied after Experience Boost", 5, hero.getXP());
  }
  // Further traits that affect XP
  // Alchemist's Pact (gain 3 XP on consuming potion)
  // Veteran (Fighter trait)
}

void testMonsterHitPoints()
{
  Monster monster(makeGenericMonsterStats(2, 10, 3), Defence(), MonsterTraits());
  ASSERT_EQUALM("A level 2 monster with 10 hit points can be created", 2, monster.getLevel());
  ASSERT_EQUALM("A level 2 monster with 10 hit points can be created", 10, monster.getHitPoints());
  monster.takeDamage(9, false);
  ASSERT_EQUALM("The monster is not defeated by a hit of 9 damage points", false, monster.isDefeated());
  ASSERT_EQUALM("It has 1 remaining hit point", 1, monster.getHitPoints());
  monster.recover(4);
  ASSERT_EQUALM("After 4 squares have been explored, it has 9 remaining hit points", 9, monster.getHitPoints());
  monster.recover(10);
  ASSERT_EQUALM("It does not recover beyond its max. HP", monster.getHitPointsMax(), monster.getHitPoints());

  monster.takeDamage(1, false);
  monster.poison(3);
  monster.recover(1);
  ASSERT_EQUALM("Poison prevents healing", 9, monster.getHitPoints());
  ASSERT_EQUALM("Poison prevents healing", 1, monster.getPoisonAmount());
  monster.recover(1);
  ASSERT_EQUALM("Poison wears off", 10, monster.getHitPoints());
  ASSERT_EQUALM("Poison wears off", false, monster.isPoisoned());

  monster.takeFireballDamage(2, false);
  ASSERT_EQUALM("Fireball does 4 damage per caster level", 8, 10 - monster.getHitPoints());
  ASSERT_EQUALM("Fireball sets monster on fire", true, monster.isBurning());
  ASSERT_EQUALM("Burn stack grows by 1", 1, monster.getBurnStackSize());
  monster.recover(4);
  ASSERT_EQUALM("Burning slows healing", 6, monster.getHitPoints());
  monster.takeFireballDamage(1, false);
  ASSERT_EQUALM("Burn stack increases damage", 5, 6 - monster.getHitPoints());
  ASSERT_EQUALM("Burn stack grows by 1", 1, monster.getBurnStackSize() - 1);
  monster.recover(2);
  ASSERT_EQUALM("Burning slows healing independently of burn stack size", 3, monster.getHitPoints());

  monster.takeDamage(0, false);
  ASSERT_EQUALM("Next hit stops burning", false, monster.isBurning());
  ASSERT_EQUALM("Burn down does damage equal to burn stack size", 2, 3 - monster.getHitPoints());
}

void testMonsterDefence()
{
  Monster monster(makeGenericMonsterStats(3, 30, 10, 1), Defence(50, 75), MonsterTraits());
  monster.takeDamage(10, false);
  ASSERT_EQUALM("Physical damage resistance is accounted for", 25, monster.getHitPoints());
  monster.stun();
  monster.takeDamage(1, false);
  ASSERT_EQUALM("Resisted damage is rounded down", 24, monster.getHitPoints());
  ASSERT_EQUALM("Taking damage clears stun", false, monster.isStunned());
  monster.takeDamage(20, true);
  ASSERT_EQUALM("Magical damage resistance is accounted for", 19, monster.getHitPoints());

  monster.corrode();
  monster.takeDamage(10, false);
  ASSERT_EQUALM("With a layer of corrosion, damage taken increases by 1", 13, monster.getHitPoints());
  monster.corrode();
  monster.takeDamage(20, true);
  ASSERT_EQUALM("With two layers of corrosion, damage taken increases by 2", 6, monster.getHitPoints());

  monster.crushResistances();
  monster.crushResistances();
  ASSERT_EQUALM("Resistances can be crushed, 3 points at a time", 69, monster.getMagicalResistPercent());

  monster.takeDamage(100, false);
  ASSERT_EQUALM("Death protection works (not dead)", false, monster.isDefeated());
  ASSERT_EQUALM("Death protection works (1 HP)", 1, monster.getHitPoints());
  ASSERT_EQUALM("Death protection is removed when used", 0, monster.getDeathProtection());
  ASSERT_EQUALM("Monster not regarded defeated when saved by death protection", false, monster.isDefeated());
  monster.takeDamage(1, false);
  ASSERT_EQUALM("Death protection no longer effective after removal", true, monster.isDefeated());
}

void testFightBasic()
{
  {
    Hero hero;
    Monster monster(makeGenericMonsterStats(3, 15, 5), {}, {});
    Melee melee(hero, monster);
    ASSERTM("Outcome summary of fight predicted correctly (safe)",
            Outcome::Summary::Safe == melee.predictOutcome().summary);
    hero.loseHitPointsOutsideOfFight(5);
    ASSERTM("Outcome summary of fight predicted correctly (hero dead)",
            Outcome::Summary::HeroDefeated == melee.predictOutcome().summary);
    hero.gainExperience(30);
    hero.loseHitPointsOutsideOfFight(hero.getHitPointsMax() - 1);
    ASSERTM("Outcome summary of fight predicted correctly (hero wins)",
            Outcome::Summary::HeroWins == melee.predictOutcome().summary);
  }
  {
    Hero hero;
    Monster monster(makeGenericMonsterStats(3, 15, 5), {}, {});
    Melee melee(hero, monster);
    ASSERT_EQUALM("Hero hitpoint loss predicted correctly", 5, melee.predictOutcome().hero->getHitPoints());
    ASSERT_EQUALM("Monster hitpoint loss predicted correctly", 10, melee.predictOutcome().monster->getHitPoints());
  }
}

void testDungeonBasic()
{
  Dungeon dungeon(3, 3);
  auto monster = std::make_shared<Monster>(makeGenericMonsterStats(3, 30, 10, 0), Defence(), MonsterTraits());
  auto monster2 = std::make_shared<Monster>(*monster);
  dungeon.add(monster, dungeon.randomFreePosition().value());
  ASSERT_EQUALM("Monsters can be added to dungeon", 1, dungeon.getMonsters().size());
  for (int i = 0; i < 8; ++i)
    dungeon.add(monster2, dungeon.randomFreePosition().value());
  ASSERTM("Finding random free position works", !dungeon.randomFreePosition());
  ASSERT_EQUALM("Finding random free position works", false, dungeon.isFree(Position(0, 0)));

  ASSERT_EQUALM("Dungeon not revealed initially", false, dungeon.isRevealed(Position(1, 1)));

  monster->takeDamage(100, false);
  ASSERT(monster->isDefeated());
  dungeon.update();
  ASSERTM("Position freed if monster is defeated", dungeon.randomFreePosition());
  auto hero = std::make_shared<Hero>();
  dungeon.setHero(hero, dungeon.randomFreePosition().value());
  ASSERTM("Hero position accounted for by randomFreePosition", !dungeon.randomFreePosition());
  ASSERT_EQUALM("Positioning hero reveals tiles", true, dungeon.isRevealed(Position(1, 1)));
}

void testPathfinding()
{
  Dungeon dungeon(10, 10);
  auto hero = std::make_shared<Hero>();
  dungeon.setHero(hero, Position(0, 0));
  ASSERT_EQUALM("Basic pathfinding (path must be revealed)", dungeon.isAccessible(Position(9, 9)), false);
  ASSERT_EQUALM("Basic pathfinding (simple unrevealed path found)", dungeon.isConnected(Position(9, 9)), true);
  for (int i = 2; i < 10; ++i)
    dungeon.reveal(Position(i, i));
  ASSERT_EQUALM("Basic pathfinding (simple revealed path found)", dungeon.isAccessible(Position(9, 9)), true);
}

bool runAllTests(int argc, char const *argv[])
{
  cute::suite s{CUTE(testHeroExperienceBasic), CUTE(testHeroExperienceAdvanced),
                CUTE(testMonsterHitPoints),    CUTE(testMonsterDefence),
                CUTE(testFightBasic),          CUTE(testDungeonBasic),
                CUTE(testPathfinding)};

  // exploration
  // path finding
  // find possible actions
  // representations
  // spells
  // inventory

  cute::xml_file_opener xmlfile(argc, argv);
  cute::xml_listener<cute::ide_listener<>> lis(xmlfile.out);
  auto runner = cute::makeRunner(lis, argc, argv);
  bool success = runner(s, "AllTests");
  return success;
}

int main(int argc, char const *argv[]) { return runAllTests(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE; }
