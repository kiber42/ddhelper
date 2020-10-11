#include "ImguiApp.hpp"

#include "Arena.hpp"
#include "HeroSelection.hpp"
#include "MonsterPool.hpp"
#include "MonsterSelection.hpp"

#include <optional>
#include <utility>
#include <vector>

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
  Arena arena;
};

DDHelperApp::DDHelperApp()
  : ImguiApp("Desktop Dungeons Simulator")
{
}

void DDHelperApp::populateFrame()
{
  auto hero = heroSelection.run();
  if (hero.has_value())
    arena.enter(std::move(*hero));

  auto customHero = heroBuilder.run();
  if (customHero.has_value())
    arena.enter(std::move(*customHero));

  monsterSelection.run();
  auto monster = monsterSelection.toArena();
  if (monster.has_value())
    arena.enter(std::move(*monster));
  monster = monsterSelection.toPool();
  if (monster.has_value())
    monsterPool.add(std::move(*monster));

  monsterBuilder.run();
  monster = monsterBuilder.toArena();
  if (monster.has_value())
    arena.enter(std::move(*monster));
  monster = monsterBuilder.toPool();
  if (monster.has_value())
    monsterPool.add(std::move(*monster));

  auto poolMonster = monsterPool.run();
  if (poolMonster.has_value())
  {
    auto oldMonster = arena.swap(std::move(poolMonster.value()));
    if (oldMonster.has_value())
      monsterPool.add(std::move(*oldMonster));
  }

  auto undoMonster = arena.run();
  if (undoMonster.has_value())
    monsterPool.add(std::move(*undoMonster));
}

int main()
{
  DDHelperApp app;
  app.run();
}
