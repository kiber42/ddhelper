#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

#include <functional>
#include <optional>
#include <utility>

struct State
{
  std::optional<Hero> hero;
  std::optional<Monster> monster;
  Monsters monsterPool;
};

using GameAction = std::function<Summary(State&)>;

std::pair<State, Outcome> applyAction(const State& initialState, const GameAction& stateUpdate, bool pessimistMode);
