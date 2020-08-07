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

inline bool canHaveMultiple(HeroStatus status)
{
  return status == HeroStatus::Corrosion || status == HeroStatus::Cursed || status == HeroStatus::Learning ||
         status == HeroStatus::Weakened;
}

inline bool isRemovable(HeroStatus status)
{
  return status != HeroStatus::BloodCurse && status != HeroStatus::Humility;
}
