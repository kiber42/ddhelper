#pragma once

#include "GodsAndBoons.hpp"
#include "State.hpp"
#include "Utils.hpp"

class Arena
{
public:
  ActionResultUI run(const State&);

private:
  ActionResultUI result;

  int selectedPopupItem;
  bool useTranslocationSeal;

  void runAttack(const State&);
  void runCastPopup(const State&);
  void runUseItemPopup(const State&);
  void runConvertItemPopup(const State&);
  void runShopPopup(const State&);
  void runFaithPopup(const State&);
  void runBoonSelection(const State& state, God deity);
  void runWorship(const State& state);
  void runUncoverTiles(const State&);
  void runPickupResource(const State&);
};
