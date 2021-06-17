#pragma once

#include "engine/Monster.hpp"

#include <optional>

namespace ui
{
  Monsters::iterator runMonsterPool(Monsters& monsters, std::optional<size_t> activeIndex);
}
