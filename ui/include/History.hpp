#pragma once

#include "State.hpp"

#include "Monster.hpp"

#include <optional>
#include <tuple>
#include <utility>
#include <vector>

class History
{
public:
  void add(State previous, ActionEntry entry);

  // Returns true if an undo was requested
  bool run();

  // Returns the previous state
  [[nodiscard]] State undo();

  [[nodiscard]] bool empty() const;

  void reset();

private:
  // Each element contains the original state and the action that was then performed on it
  std::vector<std::tuple<State, ActionEntry>> history;
};
