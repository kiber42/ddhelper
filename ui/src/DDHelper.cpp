#define SDL_MAIN_HANDLED
#include "ImguiApp.hpp"

#include "Arena.hpp"
#include "HeroSelection.hpp"
#include "History.hpp"
#include "MonsterPool.hpp"
#include "MonsterSelection.hpp"
#include "Scenario.hpp"
#include "State.hpp"

class DDHelperApp : public ImguiApp
{
public:
  DDHelperApp();

private:
  void populateFrame() override;

  HeroSelection heroSelection;
  MonsterSelection monsterSelection;
  CustomHeroBuilder heroBuilder;
  CustomMonsterBuilder monsterBuilder;
  State state;
  History history;
  Arena arena;
};

DDHelperApp::DDHelperApp()
  : ImguiApp("Desktop Dungeons Simulator")
{
}

std::optional<Scenario> runScenarioSelection()
{
  std::optional<Scenario> selection;
  ImGui::Begin("Scenario");
  ImGui::TextUnformatted("Agbaar's Academy");
  if (ImGui::Button("Slowing Part 2"))
    selection = Scenario::AgbaarsAcademySlowingPart2;
  ImGui::TextUnformatted("Hello, halflings!");
  if (ImGui::Button("Halfling Trial"))
    selection = Scenario::HalflingTrial;
  ImGui::End();
  return selection;
}

// Return initial state for a specific challenge
State prepareScenario(Scenario scenario)
{
  return { getHeroForScenario(scenario), {}, getMonstersForScenario(scenario) };
}

void DDHelperApp::populateFrame()
{
  using namespace std::string_literals;

  auto hero = heroSelection.run();
  if (!hero.has_value())
    hero = std::move(heroBuilder.run());
  if (hero.has_value())
  {
    HeroAction action = [newHero = Hero(*hero)](Hero& hero) {
      hero = Hero(newHero);
      return Summary::None;
    };
    ActionEntry entry(hero->getName() + " enters"s, std::move(action), {});
    history.add(state, std::move(entry));
    state.hero = std::move(hero);
  }

  monsterSelection.run();
  monsterBuilder.run();
  auto monster = monsterSelection.toArena();
  if (!monster.has_value())
    monster = monsterBuilder.toArena();
  if (monster.has_value())
  {
    AttackAction action = [newMonster = Monster(*monster)](Hero&, Monster& monster, Monsters&) {
      monster = Monster(newMonster);
      return Summary::None;
    };
    ActionEntry entry(monster->getName() + " enters"s, std::move(action), {});
    history.add(state, std::move(entry));
    state.monster = std::move(monster);
  }

  monster = monsterSelection.toPool();
  if (!monster.has_value())
    monster = monsterBuilder.toPool();
  if (monster.has_value())
    addMonsterToPool(std::move(*monster), state.monsterPool);

  auto poolMonster = runMonsterPool(state.monsterPool);
  if (poolMonster.has_value())
  {
    MonsterFromPool action = Monster(*poolMonster);
    ActionEntry entry(poolMonster->getName() + " enters (from pool)"s, std::move(action), {});
    history.add(state, std::move(entry));
    if (state.monster && !state.monster->isDefeated())
      addMonsterToPool(std::move(*state.monster), state.monsterPool);
    state.monster = std::move(poolMonster);
  }

  Arena::StateUpdate result = arena.run(state);
  if (result.has_value())
  {
    history.add(std::move(state), result->first);
    state = std::move(result->second);
  }

  if (history.run())
    state = history.undo();

  auto scenario = runScenarioSelection();
  if (scenario)
  {
    history.reset();
    state = prepareScenario(*scenario);
  }
}

int main()
{
  DDHelperApp app;
  app.run();
  return 0;
}
