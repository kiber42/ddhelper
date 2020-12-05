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
  showStatus(describe(hero));
}

void showStatus(const Monster& monster)
{
  showStatus(describe(monster));
}

void showStatus(const State& state)
{
  if (state.hero)
    showStatus(*state.hero);
  if (state.monster)
    showStatus(*state.monster);
}
