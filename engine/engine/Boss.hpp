#pragma once

#include "engine/Defence.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTraits.hpp"
#include "engine/StrongTypes.hpp"

enum class BossType
{
  Chzar,
  Last = Chzar
};

class Boss
{
public:
  static Monster create(BossType BossType);
  static Monster create(BossType type, HitPoints currentHitPoints);

private:
  static MonsterStats stats(BossType type);
  static Defence defence(BossType type);
  static MonsterTraits traits(BossType type);
};

constexpr const char* toString(BossType type)
{
  switch (type)
  {
  case BossType::Chzar:
    return "Chzar";
  }
}
