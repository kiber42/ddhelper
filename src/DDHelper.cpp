#include "ImguiApp.hpp"

#include "Defence.hpp"
#include "Hero.hpp"
#include "Melee.hpp"
#include "Monster.hpp"
#include "MonsterFactory.hpp"

#include <memory>
#include <optional>

class DDHelperApp : public ImguiApp
{
public:
  DDHelperApp();
  Hero heroFromForm();
  Monster monsterFromForm();

private:
  void populateFrame() override;

  int hero_data[4];
  int monster_data[4];

  std::optional<Hero> hero;
  std::optional<Monster> monster;
  std::optional<Outcome> outcome;
};

DDHelperApp::DDHelperApp()
  : ImguiApp("Desktop Dungeons Simulator")
  , hero_data{1, 10, 10, 5}
  , monster_data{1, 6, 6, 3}
{
}

void DDHelperApp::populateFrame()
{
  ImGui::Begin("Hero");
  ImGui::DragInt("Level", &hero_data[0], 0.2f, 1, 10);
  ImGui::DragInt2("HP / max", &hero_data[1], 0.5f, 0, 300);
  ImGui::DragInt("Attack", &hero_data[3], 0.5f, 0, 300);
  ImGui::End();

  ImGui::Begin("Monster");
  ImGui::DragInt("Level", &monster_data[0], 0.2f, 1, 10);
  ImGui::DragInt2("HP / max", &monster_data[1], 0.5f, 0, 300);
  ImGui::DragInt("Attack", &monster_data[3], 0.5f, 0, 300);
  ImGui::End();

  ImGui::Begin("Arena");
  if (ImGui::Button("Enter"))
  {
    printf("Getting hero from form\n");
    hero = heroFromForm();
    printf("OK\n");
    monster = monsterFromForm();
    outcome.reset();
  }
  if (hero.has_value())
  {
    ImGui::Text("Hero has %i HP", hero->getHitPoints());
    ImGui::Text("Monster has %i HP", monster->getHitPoints());
    if (ImGui::Button("Fight!"))
    {
      printf("Running outcome prediction\n");
      outcome.emplace(Melee::predictOutcome(hero.value(), monster.value()));
      printf("OK\n");
    }
    if (outcome.has_value())
    {
      switch (outcome->summary)
      {
      case Outcome::Summary::Safe:
        ImGui::Text("Safe");
        break;
      case Outcome::Summary::HeroWins:
        ImGui::Text("Win");
        break;
      case Outcome::Summary::HeroDefeated:
        ImGui::Text("Death");
        break;
      case Outcome::Summary::HeroDebuffed:
        ImGui::Text("Debuff");
        break;
      case Outcome::Summary::Error:
        break;
      }
      if (ImGui::Button("Accept outcome"))
      {
        hero = outcome->hero;
        monster = outcome->monster;
        outcome.reset();
      }
    }
  }
  ImGui::End();

  ImGui::Begin("Debug");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
              ImGui::GetIO().Framerate);
  ImGui::End();
}

Hero DDHelperApp::heroFromForm()
{
  Hero hero;
  for (int level = 1; level < hero_data[0] /* level */; ++level)
    hero.gainLevel();
  hero.modifyHitPointsMax(hero_data[2] /* max hp */ - hero.getHitPointsMax());
  hero.healHitPoints(hero_data[1] /* hp */ - hero.getHitPoints(), true);
  hero.changeBaseDamage(hero_data[3] /* damage */ - hero.getBaseDamage());
  return hero;
}

Monster DDHelperApp::monsterFromForm()
{
  Monster monster(
      makeGenericMonsterStats(monster_data[0] /* level */, monster_data[2] /* max hp */, monster_data[3] /* damage */),
      Defence(), MonsterTraits());
  if (monster_data[1] < monster_data[2])
    monster.takeDamage(monster_data[2] - monster_data[1], false);
  return monster;
}

int main()
{
  DDHelperApp app;
  app.run();
}
