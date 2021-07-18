#pragma once

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Resources.hpp"

#include <optional>
#include <vector>

enum class Scenario
{
  AgbaarsAcademySlowingPart2,
  HalflingTrial,
  TheThirdAct,
  TheMonsterMachine1,
};

Hero getHeroForScenario(Scenario scenario);
std::vector<Monster> getMonstersForScenario(Scenario scenario);

SimpleResources getResourcesForScenario(Scenario scenario);
