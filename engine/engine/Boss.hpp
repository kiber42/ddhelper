#pragma once

#include "engine/Monster.hpp"
#include "engine/StrongTypes.hpp"

#include <optional>

enum class BossType
{
  LordGobb,
  Chzar,
  Hesss,
  Last = Hesss
};

Monster create(BossType BossType);
Monster create(BossType type, std::optional<HitPoints> currentHitPoints);

constexpr const char* toString(BossType type)
{
  switch (type)
  {
  case BossType::LordGobb:
    return "Lord Gobb";
  case BossType::Chzar:
    return "Chzar";
  case BossType::Hesss:
    return "Hesss";
  }
}
