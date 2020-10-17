#pragma once

#include "State.hpp"
#include "MonsterPool.hpp"

#include <optional>

enum class Scenario
{
  AgbaarsAcademySlowingPart2
};

// Prepare state and monster pool for a specific challenge
void prepareScenario(State& state, MonsterPool& pool, Scenario scenario);

// Create ImGui frame and returns scenario to load
std::optional<Scenario> runScenarioSelection();
