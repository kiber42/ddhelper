#pragma once

#include "engine/Monster.hpp"

#include <random>

class GlowingGuardian
{
public:
  GlowingGuardian();
  bool canUseAbsolution(unsigned heroLevel, const Monsters& monsters) const;
  Monsters::iterator pickMonsterForAbsolution(unsigned heroLevel, Monsters& monsters);

private:
  std::mt19937 generator{std::random_device{}()};
};
