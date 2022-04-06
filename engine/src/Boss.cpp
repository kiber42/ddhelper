#include "engine/Boss.hpp"

namespace
{
  MonsterStats stats(BossType type)
  {
    switch (type)
    {
    case BossType::Chzar:
      return {Level{10}, 1200_HP, 50_damage};
    }
  }

  Defence defence(BossType type)
  {
    switch (type)
    {
    case BossType::Chzar:
      return {};
    }
  }

  MonsterTraits traits(BossType type)
  {
    switch (type)
    {
    case BossType::Chzar:
      return {MonsterTrait::Retaliate, MonsterTrait::Corrosive};
    }
  }
} // namespace

Monster create(BossType type)
{
  return {toString(type), stats(type), defence(type), traits(type)};
}

Monster create(BossType type, HitPoints hp)
{
  auto bossStats = stats(type);
  bossStats.setHitPoints(hp);
  return {toString(type), std::move(bossStats), defence(type), traits(type)};
}
