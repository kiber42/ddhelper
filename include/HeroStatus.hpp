#pragma once

enum class HeroStatus
{
  BloodCurse,
  // BurningStrike,
  // ConsecratedStrike,
  Corrosion,
  // Corrosive,
  // CrushingBlow,
  // CurseImmune,
  Cursed,
  // DamageReduction,
  // DeathGazeImmune,
  // DeathProtection,
  // DodgePermanent,
  // DodgeTemporary,
  // Exhausted,
  ExperienceBoost,
  FirstStrike,
  // HeavyFireball
  Humility,
  // Indulgence
  // Knockback
  Learning,
  // LifeSteal
  MagicalAttack,
  // ManaBurnImmune,
  ManaBurned,
  // Might,
  // PiercePhysical,
  // PoisonImmune,
  Poisoned,
  // Poisonous,
  // Prestige,
  Reflexes,
  // Sanguine
  // Schadenfreude,
  SlowStrike,
  // SpiritStrength,
  Weakened
  // StoneSkin
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
  case HeroStatus::ManaBurned:
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
