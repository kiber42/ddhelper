#include "History.hpp"

#include "Utils.hpp"

#include "imgui.h"

bool History::run()
{
  ImGui::Begin("History");
  int repeated = 1;
  for (unsigned i = 0; i < history.size(); ++i)
  {
    const auto& entry = std::get<HistoryEntry>(history[i]);
    if (i < history.size() - 1)
    {
      const auto next = std::get<HistoryEntry>(history[i + 1]);
      if (std::get<0>(entry) == std::get<0>(next) && std::get<2>(entry) == std::get<2>(next))
      {
        ++repeated;
        continue;
      }
    }
    ImGui::TextUnformatted(std::get<std::string>(entry).c_str());
    const auto outcome = std::get<Outcome>(entry);
    const bool debuffed = !outcome.debuffs.empty() || outcome.pietyChange < 0;
    if (outcome.summary != Summary::None || debuffed || outcome.pietyChange != 0)
    {
      const auto color = summaryColor(outcome.summary, debuffed);
      ImGui::SameLine();
      ImGui::TextColored(color, "%s", toString(outcome).c_str());
    }
    if (repeated > 1)
    {
      ImGui::SameLine();
      ImGui::Text("(x%i)", repeated);
      repeated = 1;
    }
  }

  const bool undoRequested = !history.empty() && ImGui::Button("Undo");
  ImGui::End();

  return undoRequested;
}

void History::add(State previous, HistoryEntry entry)
{
  history.emplace_back(std::tuple(std::move(previous), std::move(entry)));
}

State History::undo()
{
  assert(!history.empty());
  auto previousState = std::get<State>(std::move(history.back()));
  history.pop_back();
  return previousState;
}

bool History::empty() const
{
  return history.empty();
}

void History::reset()
{
  history.clear();
}
