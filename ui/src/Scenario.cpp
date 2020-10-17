#include "Scenario.hpp"

#include "Hero.hpp"
#include "Items.hpp"
#include "MonsterStats.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

#include "imgui.h"

Hero getHeroForScenario(Scenario scenario)
{
  switch (scenario)
  {
  case Scenario::AgbaarsAcademySlowingPart2:
  {
    Hero hero;
    hero.gainLevel();
    hero.gainExperience(4);
    hero.loseHitPointsOutsideOfFight(3);
    hero.loseAllItems();
    for (int i = 0; i < 8; ++i)
      hero.receive(Item::ManaPotion);
    hero.receiveFreeSpell(Spell::Burndayraz);
    hero.receiveFreeSpell(Spell::Weytwut);
    return hero;
  }
  }
}

void fillMonsterPoolForScenario(MonsterPool& pool, Scenario scenario)
{
  switch (scenario)
  {
  case Scenario::AgbaarsAcademySlowingPart2:
  {
    pool.add({MonsterType::Goblin, 1});
    pool.add({"Djinn level 1", {1, 19, 99, 0}, {}, MonsterTraitsBuilder().addRetaliate().get()});
    pool.add({"Zombie level 2", {2, 10, 27, 0}, {}, MonsterTraitsBuilder().addUndead().addBloodless().get()});
    pool.add({"Eeblis (level 3)", {3, 28, 999, 0}, {}, MonsterTraitsBuilder().addFirstStrike().addRetaliate().get()});
  }
  break;
  }
}

void prepareScenario(State& state, MonsterPool& pool, Scenario scenario)
{
  state.hero.emplace(getHeroForScenario(scenario));
  state.monster.reset();
  pool.reset();
  fillMonsterPoolForScenario(pool, scenario);
}

std::optional<Scenario> runScenarioSelection()
{
  std::optional<Scenario> selection;
  ImGui::Begin("Scenario");
  ImGui::TextUnformatted("Agbaar's Academy");
  if (ImGui::Button("Slowing Part 2"))
    selection = Scenario::AgbaarsAcademySlowingPart2;
  ImGui::End();
  return selection;
}
