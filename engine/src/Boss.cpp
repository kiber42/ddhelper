#include "engine/Boss.hpp"

Monster Boss::create(BossType type)
{
  return {toString(type), stats(type), defence(type), traits(type)};
}

Monster Boss::create(BossType type, HitPoints hp)
{
  auto bossStats = stats(type);
  auto hpMax = bossStats.getHitPoints();
  if (hp < hpMax)
    bossStats.loseHitPoints(hpMax - hp);
  else if (hp > hpMax)
    bossStats.healHitPoints(hp - hpMax, true);
  return {toString(type), std::move(bossStats), defence(type), traits(type)};
}

MonsterStats Boss::stats(BossType type)
{
  switch (type)
  {
  case BossType::Chzar:
    return {Level{10}, 1200_HP, 50_damage};
  }
}

Defence Boss::defence(BossType type)
{
  switch (type)
  {
  case BossType::Chzar:
    return {};
  }
}

MonsterTraits Boss::traits(BossType type)
{
  switch (type)
  {
  case BossType::Chzar:
    return {MonsterTrait::Retaliate, MonsterTrait::Corrosive};
  }
}
