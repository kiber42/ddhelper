#pragma once

#include "Hero.hpp"
#include "Monster.hpp"

#include <optional>
#include <vector>

enum class Scenario
{
  AgbaarsAcademySlowingPart2,
  HalflingTrial,
};

Hero getHeroForScenario(Scenario scenario);
std::vector<Monster> getMonstersForScenario(Scenario scenario);
