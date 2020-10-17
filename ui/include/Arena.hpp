#pragma once

#include "State.hpp"

#include <optional>
#include <utility>

class Arena
{
public:
  using StateUpdate = std::optional<std::pair<ActionEntry, State>>;
  StateUpdate run(const State& current);

private:
  int selectedPopupItem;
};
