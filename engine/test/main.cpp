#include "bandit/bandit.h"

void testHeroExperience();
void testMonsters();
void testDefenceBasics();
void testCombat();
void testSpells();
void testStatusEffects();
void testTraits();
void testInventory();
void testItems();
void testPotions();
void testResources();
void testFaith();

go_bandit([] {
  testHeroExperience();
  testMonsters();
  testDefenceBasics();
  testCombat();
  testSpells();
  testStatusEffects();
  testTraits();
  testInventory();
  testItems();
  testPotions();
  testResources();
  testFaith();
});

int main(int argc, char* argv[])
{
  return bandit::run(argc, argv);
}
