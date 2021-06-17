#include "ui/State.hpp"

namespace ui
{
  std::pair<State, Outcome> applyAction(const State& initialState, const GameAction& stateUpdate, bool pessimistMode)
  {
    State newState = initialState;
    if (pessimistMode)
      newState.hero.addStatus(HeroStatus::Pessimist);
    const auto summary = stateUpdate(newState);
    const Outcome outcome = Outcome{summary, findDebuffs(initialState.hero, newState.hero),
                                    static_cast<int>(newState.hero.getPiety()) - static_cast<int>(initialState.hero.getPiety())};
    return {std::move(newState), std::move(outcome)};
  }
} // namespace ui
