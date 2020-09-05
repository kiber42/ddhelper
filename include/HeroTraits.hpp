#pragma once

// Traits are usually available from the beginning (except for god boons)
// and cannot be removed
enum class HeroTrait
{
  Additives,
  Defiant,
};

constexpr const char* toString(HeroTrait trait)
{
  switch (trait)
  {
  case HeroTrait::Additives:
    return "Additives";
  case HeroTrait::Defiant:
    return "Defiant";
  }
}
