// IntelliSense gets confused without the relative path
#include "../include/ImguiApp.hpp"

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

  int hero_data[6];
  int monster_data[6];
  std::map<HeroStatus, int> hero_statuses;
  MonsterTraits monster_traits;

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
  ImGui::DragInt("Level", &hero_data[0], 0.1f, 1, 10);
  ImGui::DragInt2("HP / max", &hero_data[1], 0.5f, 0, 300);
  ImGui::DragInt("Attack", &hero_data[3], 0.5f, 0, 300);
  ImGui::DragInt("Physical Resistance", &hero_data[4], 0.2f, 0, 80);
  ImGui::DragInt("Magical Resistance", &hero_data[5], 0.2f, 0, 80);

  for (HeroStatus status :
       {HeroStatus::FirstStrike, HeroStatus::SlowStrike, HeroStatus::Reflexes, HeroStatus::MagicalAttack,
        HeroStatus::ExperienceBoost, HeroStatus::Poisoned, HeroStatus::ManaBurn})
  {
    bool current = hero_statuses[status] > 0;
    if (ImGui::Checkbox(toString(status), &current))
      hero_statuses[status] = current;
  }
  for (HeroStatus status : {HeroStatus::Corrosion, HeroStatus::Cursed, HeroStatus::Learning, HeroStatus::Weakened})
  {
    int current = hero_statuses[status];
    if (ImGui::InputInt(toString(status), &current))
    {
      if (current < 0)
        current = 0;
      hero_statuses[status] = current;
    }
  }
  ImGui::End();

  ImGui::Begin("Monster");
  ImGui::DragInt("Level", &monster_data[0], 0.2f, 1, 10);
  ImGui::DragInt2("HP / max", &monster_data[1], 0.5f, 0, 300);
  ImGui::DragInt("Attack", &monster_data[3], 0.5f, 0, 300);
  ImGui::DragInt("Physical Resistance", &monster_data[4], 0.2f, 0, 100);
  ImGui::DragInt("Magical Resistance", &monster_data[5], 0.2f, 0, 100);
  ImGui::Checkbox("First Strike", &monster_traits.firstStrike);
  ImGui::Checkbox("Magical Attack", &monster_traits.magicalDamage);
  ImGui::Checkbox("Retaliate", &monster_traits.retaliate);
  ImGui::Checkbox("Poisonous", &monster_traits.poisonous);
  ImGui::Checkbox("Mana Burn", &monster_traits.manaBurn);
  ImGui::Checkbox("Cursed", &monster_traits.curse);
  ImGui::Checkbox("Corrosive", &monster_traits.corrosive);
  ImGui::Checkbox("Weakening", &monster_traits.weakening);
  ImGui::Checkbox("Undead", &monster_traits.undead);
  ImGui::Checkbox("Bloodless ", &monster_traits.bloodless);
  if (ImGui::InputInt("Death Gaze %", &monster_traits.deathGazePercent))
    monster_traits.deathGazePercent = std::min(std::max(monster_traits.deathGazePercent, 0), 100);
  if (ImGui::InputInt("Life Steal %", &monster_traits.lifeStealPercent))
    monster_traits.lifeStealPercent = std::min(std::max(monster_traits.lifeStealPercent, 0), 100);
  ImGui::End();

  ImGui::Begin("Arena");
  if (ImGui::Button("Enter"))
  {
    hero = heroFromForm();
    monster = monsterFromForm();
    outcome.reset();
  }
  if (hero.has_value())
  {
    ImGui::Text("Hero has %i HP", hero->getHitPoints());
    ImGui::Text("Monster has %i HP", monster->getHitPoints());
    if (ImGui::Button("Fight!"))
      outcome.emplace(Melee::predictOutcome(hero.value(), monster.value()));
    if (outcome.has_value())
    {
      ImGui::Text("%s", toString(outcome->summary));
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
  hero.setPhysicalResistPercent(hero_data[4]);
  hero.setMagicalResistPercent(hero_data[5]);
  for (const auto& [status, intensity] : hero_statuses)
    for (int i = 0; i < intensity; ++i)
      hero.addStatus(status);
  return hero;
}

Monster DDHelperApp::monsterFromForm()
{
  Monster monster(makeGenericMonsterStats(monster_data[0] /* level */, monster_data[2] /* max hp */,
                                          monster_data[3] /* damage */, 0 /* death protection */),
                  {monster_data[4] /* physical resistance */, monster_data[5] /* magical resistance */},
                  monster_traits);
  if (monster_data[1] < monster_data[2])
    monster.takeDamage(monster_data[2] - monster_data[1], false);
  return monster;
}

int main()
{
  DDHelperApp app;
  app.run();
}
