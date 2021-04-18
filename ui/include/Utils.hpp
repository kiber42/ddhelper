#pragma once

#include "State.hpp"

#include "Hero.hpp"
#include "Monster.hpp"
#include "Outcome.hpp"

#include "imgui.h"

#include <optional>
#include <utility>

extern ImVec4 colorSafe;
extern ImVec4 colorWin;
extern ImVec4 colorDeath;
extern ImVec4 colorLevelUp;
extern ImVec4 colorNotPossible;

extern ImVec4 colorDebuffedSafe;
extern ImVec4 colorDebuffedWin;
extern ImVec4 colorDebuffedLevelUp;

extern ImVec4 colorUnavailable;

const ImVec4& summaryColor(Summary, bool debuffed);
const ImVec4& outcomeColor(const Outcome& outcome);

void createToolTip(std::function<void()> createToolTipContents);
void disabledButton(const char* label, const char* tooltip = "");

void showStatus(const Hero&);
void showStatus(const Monster&);
void showStatus(const State&);

using ActionResultUI = std::optional<std::pair<std::string, GameAction>>;

void addActionButton(const State& state,
                     std::string buttonAndHistoryText,
                     const GameAction& action,
                     ActionResultUI& result);

void addActionButton(const State& state,
                     std::string buttonText,
                     bool smallButton,
                     std::string historyTitle,
                     const GameAction& action,
                     ActionResultUI& result);

bool addPopupAction(const State& state,
                    std::string itemLabel,
                    std::string historyTitle,
                    const GameAction& action,
                    bool wasSelected,
                    ActionResultUI& result);
