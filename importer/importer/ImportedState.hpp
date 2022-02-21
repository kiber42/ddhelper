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
    std::optional<HealthInfo> health;
    std::uint32_t hash;
  };

  struct ImportedState
  {
    std::vector<MonsterInfo> monsterInfos;
  };

} // namespace importer
