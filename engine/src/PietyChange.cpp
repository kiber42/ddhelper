#include "engine/PietyChange.hpp"

PietyChange::PietyChange(int deltaPoints)
  : values({deltaPoints})
  , pact(std::nullopt)
  , jehora(false)
{
}

PietyChange::PietyChange(Pact activated)
  : values()
  , pact(activated)
  , jehora(false)
{
}

PietyChange::PietyChange(JehoraTriggered)
  : values()
  , pact(std::nullopt)
  , jehora(true)
{
}

const std::vector<int>& PietyChange::operator()() const
{
  return values;
}

std::optional<Pact> PietyChange::activatedPact() const
{
  return pact;
}

bool PietyChange::randomJehoraEvent() const
{
  return jehora;
}

PietyChange& PietyChange::operator+=(const PietyChange& other)
{
  for (auto otherValue : other.values)
    values.emplace_back(otherValue);
  if (!pact.has_value())
    pact = other.pact;
  jehora |= other.jehora;
  return *this;
}

PietyChange& PietyChange::operator+=(int deltaPoints)
{
  values.emplace_back(deltaPoints);
  return *this;
}
