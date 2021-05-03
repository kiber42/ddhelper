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
    std::optional<Hero> run();
    Hero get() const;

  private:
    DungeonSetup setup{HeroClass::Fighter, HeroRace::Human};
    int level{1};
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
