#include "bandit/bandit.h"

void testHeroExperience();
void testMonsterBasics();
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
  testMonsterBasics();
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
