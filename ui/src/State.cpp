#include "State.hpp"

namespace ui
{
  std::pair<State, Outcome> applyAction(const State& initialState, const GameAction& stateUpdate, bool pessimistMode)
  {
    State newState = initialState;
    if (pessimistMode)
      newState.hero.addStatus(HeroStatus::Pessimist);
    const auto summary = stateUpdate(newState);
    const Outcome outcome = Outcome{summary, findDebuffs(initialState.hero, newState.hero),
                                    newState.hero.getPiety() - initialState.hero.getPiety()};
    return {std::move(newState), std::move(outcome)};
  }
} // namespace ui
