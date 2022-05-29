#pragma once

#include "ui/State.hpp"
#include "ui/Utils.hpp"

namespace ui
{
  class RunHeuristics
  {
  public:
    void update(const State& state);
    void show() const;

  private:
    std::vector<std::string> output;
  };
} // namespace ui
