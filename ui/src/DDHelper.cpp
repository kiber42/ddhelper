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
  MonsterPool monsterPool;
  State state;
  History history;
  Arena arena;
};

DDHelperApp::DDHelperApp()
  : ImguiApp("Desktop Dungeons Simulator")
{
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
      return Summary::Safe;
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
    AttackAction action = [newMonster = Monster(*monster)](Hero&, Monster& monster) {
      monster = Monster(newMonster);
      return Summary::Safe;
    };
    ActionEntry entry(monster->getName() + " enters"s, std::move(action), {});
    history.add(state, std::move(entry));
    state.monster = std::move(monster);
  }

  monster = monsterSelection.toPool();
  if (!monster.has_value())
    monster = monsterBuilder.toPool();
  if (monster.has_value())
    monsterPool.add(std::move(*monster));

  auto poolMonster = monsterPool.run();
  if (poolMonster.has_value())
  {
    MonsterFromPool action = Monster(*poolMonster);
    ActionEntry entry(poolMonster->getName() + " enters (from pool)"s, std::move(action), {});
    history.add(state, std::move(entry));
    if (state.monster && !state.monster->isDefeated())
      monsterPool.add(std::move(*state.monster));
    state.monster = std::move(poolMonster);
  }

  Arena::StateUpdate result = arena.run(state);
  if (result.has_value())
  {
    history.add(std::move(state), result->first);
    state = std::move(result->second);
  }

  if (history.run())
  {
    auto undoInfo = history.undo();
    state = std::move(undoInfo.first);
    if (undoInfo.second)
      monsterPool.add(std::move(*undoInfo.second));
  }

  auto scenario = runScenarioSelection();
  if (scenario)
  {
    history.reset();
    prepareScenario(state, monsterPool, *scenario);
  }
}

int main()
{
  DDHelperApp app;
  app.run();
}
