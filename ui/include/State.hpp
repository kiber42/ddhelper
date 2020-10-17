#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

#include <functional>
#include <optional>
#include <tuple>
#include <variant>

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
