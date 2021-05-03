#pragma once

#include "engine/DungeonSetup.hpp"
#include "engine/Hero.hpp"
#include "engine/HeroClass.hpp"

#include <array>
#include <map>
#include <optional>

namespace ui
{
  class HeroSelection
  {
  public:
    HeroSelection();
    std::optional<Hero> run();
    Hero get() const;

  private:
    HeroClass selectedClass;
    HeroRace selectedRace;
    DungeonSetup preparations;
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
} // namespace ui
