#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

#include <cassert>
#include <functional>
#include <optional>
#include <utility>

struct State
{
  std::optional<Hero> hero;
  Monsters monsterPool;
  int activeMonster = -1;

  Monster* monster()
  {
    assert(activeMonster < static_cast<int>(monsterPool.size()));
    return activeMonster >= 0 ? &monsterPool[activeMonster] : nullptr;
  }

  const Monster* monster() const
  {
    assert(activeMonster < static_cast<int>(monsterPool.size()));
    return activeMonster >= 0 ? &monsterPool[activeMonster] : nullptr;
  }
};

using GameAction = std::function<Summary(State&)>;

std::pair<State, Outcome> applyAction(const State& initialState, const GameAction& stateUpdate, bool pessimistMode);
