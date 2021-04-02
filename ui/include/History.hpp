#pragma once

#include "State.hpp"

#include "Outcome.hpp"

#include <string>
#include <tuple>
#include <vector>

using HistoryEntry = std::tuple<std::string, GameAction, Outcome>;

class History
{
public:
  // Produce imgui history frame. Returns true if an undo was requested
  bool run();

  void add(State previous, HistoryEntry entry);

  // Returns the previous state
  [[nodiscard]] State undo();

  // Returns the last history entry
  [[nodiscard]] const HistoryEntry& peek_back();

  [[nodiscard]] bool empty() const;

  void reset();

private:
  // Each element contains the original state and the action that was then performed on it
  std::vector<std::tuple<State, HistoryEntry>> history;
};
