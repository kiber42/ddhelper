#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Resources.hpp"

#include <optional>
#include <vector>

enum class Scenario
{
  AgbaarsAcademySlowingPart2,
  HalflingTrial,
  TheThirdAct,
};

Hero getHeroForScenario(Scenario scenario);
std::vector<Monster> getMonstersForScenario(Scenario scenario);

SimpleResources getResourcesForScenario(Scenario scenario);
