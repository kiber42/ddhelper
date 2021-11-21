#include "bandit/bandit.h"

void testHeroExperience();
void testMonsters();
void testDefenceBasics();
void testCombat();
void testSpells();
void testStatusEffects();
void testHeroTraits();
void testInventory();
void testResources();
void testPotions();
void testFaith();

go_bandit([] {
  testHeroExperience();
  testMonsters();
  testDefenceBasics();
  testCombat();
  testSpells();
  testStatusEffects();
  testHeroTraits();
  testInventory();
  testResources();
  testPotions();
  testFaith();
});

int main(int argc, char* argv[])
{
  return bandit::run(argc, argv);
}
