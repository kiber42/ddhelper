#pragma once

#include "Hero.hpp"
#include "HeroClass.hpp"

#include <array>
#include <map>
#include <optional>

class HeroSelection
{
public:
  HeroSelection();
  std::optional<Hero> run();
  Hero get() const;

private:
  HeroClass selectedClass;
  HeroRace selectedRace;
  int selectedClassIndex;
  int level;
};

class CustomHeroBuilder
{
public:
  CustomHeroBuilder();
  std::optional<Hero> run();
  Hero get() const;

private:
  std::array<int, 9> data;
  std::map<HeroStatus, int> statuses;
};