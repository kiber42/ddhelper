#include "engine/Boss.hpp"

namespace
{
  MonsterStats stats(BossType type)
  {
    switch (type)
    {
    case BossType::LordGobb:
      return {Level{8}, 176_HP, 49_damage};
    case BossType::Chzar:
      return {Level{10}, 1200_HP, 50_damage};
    case BossType::Hesss:
      return {Level{10}, 5000_HP, 75_damage};
    }
  }

  Defence defence(BossType type)
  {
    switch (type)
    {
    case BossType::LordGobb:
      return {20_physicalresist, 20_magicalresist};
    case BossType::Chzar:
      return {};
    case BossType::Hesss:
      return {};
    }
  }

  MonsterTraits traits(BossType type)
  {
    switch (type)
    {
    case BossType::LordGobb:
      return {MonsterTrait::FirstStrike};
    case BossType::Chzar:
      return {MonsterTrait::Retaliate, MonsterTrait::Corrosive};
    case BossType::Hesss:
      return {MonsterTrait::Cowardly, MonsterTrait::Weakening, MonsterTrait::CurseBearer, MonsterTrait::Poisonous,
              MonsterTrait::ManaBurn};
    }
  }
} // namespace

Monster create(BossType type)
{
  return {toString(type), stats(type), defence(type), traits(type)};
}

Monster create(BossType type, std::optional<HitPoints> hp)
{
  auto bossStats = stats(type);
  if (hp)
    bossStats.setHitPoints(*hp);
  return {toString(type), std::move(bossStats), defence(type), traits(type)};
}
