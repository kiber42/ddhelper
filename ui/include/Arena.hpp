#pragma once

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

#include <optional>
#include <tuple>
#include <vector>

class Arena
{
public:
  void enter(Hero&&);
  void enter(Monster&&);
  [[nodiscard]] std::optional<Monster> swap(Monster&&);
  std::optional<Monster> run();

private:
  std::optional<Hero> hero;
  std::optional<Monster> monster;

  using HistoryItem =
      std::tuple<std::string, Outcome::Summary, Outcome::Debuffs, std::optional<Hero>, std::optional<Monster>, bool>;
  std::vector<HistoryItem> history;

  int selectedPopupItem;
};
