#pragma once

#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"
#include "engine/StrongTypes.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace importer
{
  struct TilePosition
  {
    short x;
    short y;
  };

  using HealthInfo = std::pair<short, short>;

  struct MonsterInfo
  {
    TilePosition position;
    MonsterType type;
    Level level;

    // A health bar is shown for monsters that do not have full health.
    // It is also shown for monsters with more than 100% health, unless they are slowed.
    bool hasHealthBar;
    std::optional<HealthInfo> health;

    std::uint32_t hash;

    Monster toMonster(DungeonMultiplier multiplier) const;
  };

  struct ImportedState
  {
    std::vector<MonsterInfo> monsterInfos;
  };

} // namespace importer
