#pragma once

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Outcome.hpp"
#include "engine/Resources.hpp"

#include <cassert>
#include <functional>
#include <utility>

namespace ui
{
  struct State
  {
    Hero hero{};
    Monsters monsterPool{};
    std::optional<std::size_t> activeMonster;
    MapResources resources{};

    Monster* monster()
    {
      if (!activeMonster)
        return nullptr;
      assert(*activeMonster < monsterPool.size());
      return &monsterPool[*activeMonster];
    }

    const Monster* monster() const
    {
      if (!activeMonster)
        return nullptr;
      assert(*activeMonster < monsterPool.size());
      return &monsterPool[*activeMonster];
    }
  };

  using GameAction = std::function<Summary(State&)>;

  std::pair<State, Outcome> applyAction(const State& initialState, const GameAction& stateUpdate, bool pessimistMode);
} // namespace ui
