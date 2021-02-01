#include "Utils.hpp"

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
  if (items.size() > 7)
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
  if (state.hero)
    showStatus(*state.hero);
  if (state.activeMonster >= 0)
    showStatus(state.monsterPool[state.activeMonster]);
}
