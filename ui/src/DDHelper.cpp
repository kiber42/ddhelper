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
  return {getHeroForScenario(scenario), {}, getMonstersForScenario(scenario)};
}

void DDHelperApp::populateFrame()
{
  using namespace std::string_literals;

  auto applyUndoable = [&](std::string title, GameAction stateUpdate) {
    auto [newState, outcome] = applyAction(state, stateUpdate, false);
    history.add(std::move(state), {std::move(title), std::move(stateUpdate), std::move(outcome)});
    state = std::move(newState);
  };

  auto hero = heroSelection.run();
  if (!hero)
    hero = heroBuilder.run();
  if (hero)
  {
    std::string title = hero->getName() + " enters"s;
    applyUndoable(std::move(title), [newHero = Hero(*hero)](State& state) {
      state.hero = newHero;
      return Summary::None;
    });
  }

  monsterSelection.run();
  monsterBuilder.run();
  auto monster = monsterSelection.toArena();
  if (!monster)
    monster = monsterBuilder.toArena();
  if (monster)
  {
    std::string title = monster->getName() + " enters"s;
    applyUndoable(std::move(title), [newMonster = std::move(*monster)](State& state) {
      if (state.monster)
        state.monsterPool.emplace_back(std::move(*state.monster));
      state.monster = newMonster;
      return Summary::None;
    });
  }

  monster = monsterSelection.toPool();
  if (!monster)
    monster = monsterBuilder.toPool();
  if (monster)
  {
    std::string title = monster->getName() + " added to pool"s;
    applyUndoable(std::move(title), [newMonster = std::move(*monster)](State& state) {
      state.monsterPool.emplace_back(newMonster);
      return Summary::None;
    });
  }

  auto poolMonster = runMonsterPool(state.monsterPool);
  if (poolMonster != end(state.monsterPool))
  {
    std::string title = poolMonster->getName() + " enters from pool"s;
    applyUndoable(std::move(title), [poolIndex = std::distance(begin(state.monsterPool), poolMonster)](State& state) {
      if (state.monster && !state.monster->isDefeated())
        state.monsterPool.emplace_back(*state.monster);
      auto poolMonster = begin(state.monsterPool) + poolIndex;
      state.monster = *poolMonster;
      state.monsterPool.erase(poolMonster);
      return Summary::None;
    });
  }

  auto result = arena.run(state);
  if (result)
    applyUndoable(std::move(result->first), std::move(result->second));

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
