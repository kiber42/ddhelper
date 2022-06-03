#include "ui/Utils.hpp"

#include "imgui.h"

namespace ui
{
  ImVec4 colorSafe(0, 0.5f, 0, 1);
  ImVec4 colorWin(0.2f, 1, 0.2f, 1);
  ImVec4 colorDeath(1, 0, 0, 1);
  ImVec4 colorLevelUp(0.5f, 1, 0.5f, 1);
  ImVec4 colorNotPossible(0, 0, 0, 1);
  ImVec4 colorDebuffedSafe(0, 0.5f, 0.5f, 1);
  ImVec4 colorDebuffedWin(0, 0.8f, 0.8f, 1);
  ImVec4 colorDebuffedLevelUp(0, 1, 1, 1);
  ImVec4 colorUnavailable(0.5f, 0.5f, 0.5f, 1);

  const ImVec4& summaryColor(Summary summary, bool debuffed)
  {
    switch (summary)
    {
    case Summary::None:
    case Summary::Safe:
      return debuffed ? colorDebuffedSafe : colorSafe;
    case Summary::Win:
      return debuffed ? colorDebuffedWin : colorWin;
    case Summary::Death:
    case Summary::Petrified:
      return colorDeath;
    case Summary::LevelUp:
      return debuffed ? colorDebuffedLevelUp : colorLevelUp;
    case Summary::NotPossible:
      return colorNotPossible;
    }
  }

  const ImVec4& outcomeColor(const Outcome& outcome)
  {
    return summaryColor(outcome.summary, !outcome.debuffs.empty() || outcome.pietyChange < 0);
  }

  void createToolTip(std::function<void()> createToolTipContents)
  {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    createToolTipContents();
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }

  void disabledButton(const char* label, const char* tooltip)
  {
    ImGui::PushStyleColor(0, colorUnavailable);
    ImGui::Button(label);
    ImGui::PopStyleColor();
    if (strlen(tooltip) && ImGui::IsItemHovered())
      createToolTip([&] { ImGui::TextUnformatted(tooltip); });
  }

  void disabledSmallButton(const char* label, const char* tooltip)
  {
    ImGui::PushStyleColor(0, colorUnavailable);
    ImGui::SmallButton(label);
    ImGui::PopStyleColor();
    if (strlen(tooltip) && ImGui::IsItemHovered())
      createToolTip([&] { ImGui::TextUnformatted(tooltip); });
  }

  void showStatus(const std::vector<std::string>& description)
  {
    bool first = true;
    for (const auto& line : description)
    {
      if (!first)
        ImGui::Text("  %s", line.c_str());
      else
        ImGui::Text("%s", line.c_str());
      first = false;
    }
  }

  void showStatus(const Hero& hero)
  {
    auto items = describe(hero);
    if (items.size() >= 8)
    {
      items[1] = items[1] + "  " + items[2] + "  " + items[3] + "  " + items[4];
      items[2] = items[5] + "  " + items[6] + "  " + items[7];
      items.erase(items.begin() + 3, items.begin() + 8);
    }
    showStatus(items);
  }

  void showStatus(const Monster& monster)
  {
    auto items = describe(monster);
    if (items.size() >= 3)
    {
      items[0] += " has " + items[1] + " and does " + items[2];
      items.erase(items.begin() + 1, items.begin() + 3);
    }
    showStatus(items);
  }

  void showStatus(const State& state)
  {
    showStatus(state.hero);
    if (state.activeMonster)
      showStatus(state.monsterPool[*state.activeMonster]);
  }

  void showPredictedOutcomeTooltip(const State& initialState, const GameAction& stateUpdate)
  {
    const auto result = applyAction(initialState, stateUpdate, true);
    const auto& newState = result.first;
    const auto& outcome = result.second;
    if (outcome.summary == Summary::None)
      return;
    createToolTip([&] {
      const auto outcomeStr = toString(outcome);
      if (!outcomeStr.empty())
        ImGui::TextColored(outcomeColor(outcome), "%s", outcomeStr.c_str());
      showStatus(newState);
    });
  }

  void
  addAction(const State& state, std::string title, const GameAction& action, bool activated, ActionResultUI& result)
  {
    if (activated)
      result.emplace(std::pair{std::move(title), std::move(action)});
    else if (ImGui::IsItemHovered())
      showPredictedOutcomeTooltip(state, action);
  }

  void addActionButton(const State& state,
                       std::string buttonAndHistoryText,
                       const GameAction& action,
                       ActionResultUI& result)
  {
    addActionButton(state, buttonAndHistoryText, false, buttonAndHistoryText, action, result);
  }

  void addActionButton(const State& state,
                       std::string buttonText,
                       bool smallButton,
                       std::string historyTitle,
                       const GameAction& action,
                       ActionResultUI& result)
  {
    ImGui::PushID(historyTitle.c_str());
    const bool buttonPressed = smallButton ? ImGui::SmallButton(buttonText.c_str()) : ImGui::Button(buttonText.c_str());
    ImGui::PopID();
    addAction(state, std::move(historyTitle), action, buttonPressed, result);
  }

  bool addPopupAction(const State& state,
                      std::string itemLabel,
                      std::string historyTitle,
                      const GameAction& action,
                      bool wasSelected,
                      ActionResultUI& result)
  {
    const bool mouseDown = ImGui::IsAnyMouseDown();
    const bool becameSelected = (ImGui::Selectable(itemLabel.c_str()) || (ImGui::IsItemHovered() && mouseDown));
    addAction(state, std::move(historyTitle), action, wasSelected && !mouseDown, result);
    return becameSelected;
  }
} // namespace ui
