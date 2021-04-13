#pragma once

#include "Monster.hpp"

#include <random>

class GlowingGuardian
{
public:
  GlowingGuardian();
  bool canUseAbsolution(int heroLevel, const Monsters& monsters) const;
  Monsters::iterator pickMonsterForAbsolution(int heroLevel, Monsters& monsters);

private:
  std::mt19937 generator{std::random_device{}()};
};
