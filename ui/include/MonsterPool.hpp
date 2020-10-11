#pragma once

#include "Monster.hpp"

#include <optional>
#include <vector>

class MonsterPool
{
public:
  void add(Monster&&);
  std::optional<Monster> run();

private:
  std::vector<Monster> monsters;
};

