#include "State.hpp"

std::pair<State, Outcome> applyAction(const State& initialState, const GameAction& stateUpdate, bool pessimistMode)
{
  State newState = initialState;
  if (pessimistMode)
    newState.hero->addStatus(HeroStatus::Pessimist);
  const auto summary = stateUpdate(newState);
  const Outcome outcome = initialState.hero && newState.hero
                              ? Outcome{summary, findDebuffs(*initialState.hero, *newState.hero),
                                        newState.hero->getPiety() - initialState.hero->getPiety()}
                              : Outcome{summary, {}, 0};
  return {std::move(newState), std::move(outcome)};
}
