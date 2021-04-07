#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

#include "ResourcesUI.hpp"

#include <cassert>
#include <functional>
#include <utility>

struct State
{
  Hero hero{};
  Monsters monsterPool{};
  int activeMonster = -1;
  MapResources resources{20};

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
