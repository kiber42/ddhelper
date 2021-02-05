#pragma once

#include "History.hpp"
#include "State.hpp"

#include <optional>
#include <utility>

class Arena
{
public:
  using ArenaResult = std::optional<std::pair<std::string, GameAction>>;
  ArenaResult run(const State&);

private:
  ArenaResult result;
  int selectedPopupItem;

  void addAction(const State&, std::string title, const GameAction&, bool activated);
  void addActionButton(const State&, std::string title, const GameAction&);
  bool addPopupAction(const State&, std::string label, std::string historyTitle, const GameAction&, bool wasSelected);

  void runAttack(const State&);
  void runCastPopup(const State&);
  void runUseItemPopup(const State&);
  void runConvertItemPopup(const State&);
  void runShopPopup(const State&);
  void runFaithPopup(const State&);
  void runUncoverTiles(const State&);
  void runFindPopup(const State&);
};
