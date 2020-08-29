#pragma once

enum class HeroStatus
{
  BloodCurse,
  Corrosion,
  Cursed,
  ExperienceBoost,
  FirstStrike,
  Humility,
  Learning,
  MagicalAttack,
  ManaBurn,
  Poisoned,
  Reflexes,
  SlowStrike,
  Weakened
};

constexpr const char* toString(HeroStatus status)
{
  switch (status)
  {
  case HeroStatus::BloodCurse:
    return "Blood Curse";
  case HeroStatus::Corrosion:
    return "Corrosion";
  case HeroStatus::Cursed:
    return "Cursed";
  case HeroStatus::ExperienceBoost:
    return "Experience Boost";
  case HeroStatus::FirstStrike:
    return "First Strike";
  case HeroStatus::Humility:
    return "Humility";
  case HeroStatus::Learning:
    return "Learning";
  case HeroStatus::MagicalAttack:
    return "Magical Attack";
  case HeroStatus::ManaBurn:
    return "Mana Burn";
  case HeroStatus::Poisoned:
    return "Poisoned";
  case HeroStatus::Reflexes:
    return "Reflexes";
  case HeroStatus::SlowStrike:
    return "Slow Strike";
  case HeroStatus::Weakened:
    return "Weakened";
  }
}

constexpr bool canHaveMultiple(HeroStatus status)
{
  return status == HeroStatus::Corrosion || status == HeroStatus::Cursed || status == HeroStatus::Learning ||
         status == HeroStatus::Weakened;
}

constexpr bool isRemovable(HeroStatus status)
{
  return status != HeroStatus::BloodCurse && status != HeroStatus::Humility;
}
