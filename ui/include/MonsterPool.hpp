#pragma once

#include "Monster.hpp"

#include <optional>
#include <vector>

class MonsterPool
{
public:
  void add(Monster);
  void assign(std::vector<Monster> newMonsters);
  std::optional<Monster> run();
  void reset();

private:
  std::vector<Monster> monsters;
};
