#pragma once

#include "engine/Monster.hpp"
#include "engine/StrongTypes.hpp"

#include <optional>

enum class BossType
{
  Chzar,
  Last = Chzar
};

Monster create(BossType BossType);
Monster create(BossType type, std::optional<HitPoints> currentHitPoints);

constexpr const char* toString(BossType type)
{
  switch (type)
  {
  case BossType::Chzar:
    return "Chzar";
  }
}
