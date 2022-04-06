#pragma once

#include "engine/Monster.hpp"
#include "engine/StrongTypes.hpp"

enum class BossType
{
  Chzar,
  Last = Chzar
};

Monster create(BossType BossType);
Monster create(BossType type, HitPoints currentHitPoints);

constexpr const char* toString(BossType type)
{
  switch (type)
  {
  case BossType::Chzar:
    return "Chzar";
  }
}
