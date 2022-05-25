#include "importer/ImportedState.hpp"

namespace importer
{
  Monster MonsterInfo::toMonster(DungeonMultiplier multiplier) const
  {
    if (health)
    {
      auto [hp, hpMax] = *health;
      auto stats = MonsterStats{type, level, multiplier};
      stats.setHitPointsMax(HitPoints{hpMax});
      stats.setHitPoints(HitPoints{hp});
      return {Monster::makeName(type, level), std::move(stats), Defence{type}, MonsterTraits{type}};
    }
    else
      return {type, level, multiplier};
  }
} // namespace importer
