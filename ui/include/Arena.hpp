#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

#include <functional>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

using HeroAction = std::function<Summary(Hero&)>;
using AttackAction = std::function<Summary(Hero&, Monster&)>;
using MonsterFromPool = Monster;

using AnyAction = std::variant<HeroAction, AttackAction, MonsterFromPool, std::monostate>;

using ActionEntry = std::tuple<std::string, AnyAction, Outcome>;

struct State
{
  std::optional<Hero> hero;
  std::optional<Monster> monster;
};

class History
{
public:
  void add(State previous, ActionEntry entry);

  // Returns true if an undo was requested
  bool run();

  [[nodiscard]] bool empty() const;

  // Returns the previous state and optionally a Monster to be returned to the Pool
  [[nodiscard]] std::pair<State, std::optional<Monster>> undo();

private:
  // Each element contains the original state and the action that was then performed on it
  std::vector<std::tuple<State, ActionEntry>> history;
};

class Arena
{
public:
  using StateUpdate = std::optional<std::pair<ActionEntry, State>>;
  StateUpdate run(const State& current);

private:
  int selectedPopupItem;
};
