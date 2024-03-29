#pragma once

#include "engine/GodsAndBoons.hpp"

class Hero;

#include <optional>
#include <vector>

struct JehoraTriggered
{
};

struct [[nodiscard]] PietyChange
{
public:
  PietyChange(int deltaPoints = 0);
  PietyChange(Pact activated);
  PietyChange(JehoraTriggered);
  const std::vector<int>& operator()() const;
  std::optional<Pact> activatedPact() const;
  bool randomJehoraEvent() const;
  PietyChange& operator+=(const PietyChange&);
  PietyChange& operator+=(int deltaPoints);

private:
  std::vector<int> values;
  std::optional<Pact> pact;
  bool jehora{false};
};

class PietyCollection
{
  public:
    PietyCollection(Hero& hero);
    void operator()(const PietyChange& pietyChange);
    PietyCollection(const Hero&) = delete;
    PietyCollection& operator=(const Hero&) = delete;
    ~PietyCollection();
  private:
    Hero& hero;
};
