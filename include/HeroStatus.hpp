#pragma once

enum class HeroStatus
{
  // BurningStrike,
  ConsecratedStrike,
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
  HeavyFireball,
  // Indulgence
  // Knockback
  Learning,
  // LifeSteal
  MagicalAttack,
  // ManaBurnImmune,
  ManaBurned,
  Might, // +30% dmg, +3% erode resistances
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
  case HeroStatus::ConsecratedStrike:
    return "Consecrated Strike";
  case HeroStatus::Corrosion:
    return "Corrosion";
  case HeroStatus::Cursed:
    return "Cursed";
  case HeroStatus::ExperienceBoost:
    return "Experience Boost";
  case HeroStatus::FirstStrike:
    return "First Strike";
  case HeroStatus::HeavyFireball:
    return "Heavy Fireball";
  case HeroStatus::Learning:
    return "Learning";
  case HeroStatus::MagicalAttack:
    return "Magical Attack";
  case HeroStatus::ManaBurned:
    return "Mana Burn";
  case HeroStatus::Might:
    return "Might";
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
