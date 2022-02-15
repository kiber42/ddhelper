#pragma once

#include "solver/GameState.hpp"

#include <string>

class StateFitnessRating
{
  public:
    const int GAME_WON = 10000;
    const int GAME_LOST = -100000;
    virtual int operator()(const GameState&) const = 0;
    virtual std::string explain(const GameState&) const { return ""; }
};

class StateFitnessRating1 : public StateFitnessRating
{
  public:
    int operator()(const GameState&) const override final;
    std::string explain(const GameState&) const override final;
};

class StateFitnessRating2 : public StateFitnessRating
{
  public:
    int operator()(const GameState&) const override final;
    std::string explain(const GameState&) const override final { return ""; };
};
