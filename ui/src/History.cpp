#include "History.hpp"

#include "Utils.hpp"

#include "imgui.h"

void History::add(State previous, ActionEntry entry)
{
  history.emplace_back(std::tuple(std::move(previous), std::move(entry)));
}

bool History::run()
{
  ImGui::Begin("History");
  int repeated = 1;
  for (unsigned i = 0; i < history.size(); ++i)
  {
    const auto& entry = std::get<ActionEntry>(history[i]);
    if (i < history.size() - 1)
    {
      const auto next = std::get<ActionEntry>(history[i + 1]);
      if (std::get<0>(entry) == std::get<0>(next) && std::get<2>(entry) == std::get<2>(next))
      {
        ++repeated;
        continue;
      }
    }
    ImGui::TextUnformatted(std::get<std::string>(entry).c_str());
    const auto outcome = std::get<Outcome>(entry);
    if (outcome.summary != Summary::Safe)
    {
      const auto color = summaryColor(outcome.summary, !outcome.debuffs.empty());
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

bool History::empty() const
{
  return history.empty();
}

std::pair<State, std::optional<Monster>> History::undo()
{
  assert(!history.empty());
  std::optional<Monster> undoMonster;
  auto& restore = history.back();
  AnyAction& action = std::get<AnyAction>(std::get<ActionEntry>(restore));
  if (auto monster = std::get_if<MonsterFromPool>(&action))
    undoMonster.emplace(std::move(*monster));
  auto previousState = std::move(std::get<State>(restore));
  history.pop_back();
  return std::pair{std::move(previousState), std::move(undoMonster)};
}
