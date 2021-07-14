#pragma once

#include "engine/Hero.hpp"
#include "engine/Monster.hpp"
#include "engine/Outcome.hpp"
#include "engine/Resources.hpp"

#include <cassert>
#include <functional>
#include <optional>
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

    void removeDefeatedMonsters()
    {
      size_t index = 0;
      while (index < monsterPool.size())
      {
        if (monsterPool[index].isDefeated())
        {
          monsterPool.erase(std::next(begin(monsterPool), index));
          if (activeMonster && *activeMonster > index)
            activeMonster = *activeMonster - 1;
        }
        else
          ++index;
      }
    }

    void heroAndMonsterRecovery(unsigned numTiles)
    {
      hero.recover(numTiles, monsterPool);
      for (auto& monster : monsterPool)
        monster.recover(numTiles);
    }
  };

  using GameAction = std::function<Summary(State&)>;

  std::pair<State, Outcome> applyAction(const State& initialState, const GameAction& stateUpdate, bool pessimistMode);
} // namespace ui
