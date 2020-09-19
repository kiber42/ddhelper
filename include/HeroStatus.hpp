#pragma once

enum class HeroStatus
{
  // BurningStrike,
  ConsecratedStrike,
  Corrosion,
  // Corrosive,
  CrushingBlow,
  Cursed,
  CurseImmune,
  DamageReduction,
  // DeathGazeImmune,
  DeathProtection,
  DodgePermanent,
  DodgeTemporary,
  // Exhausted,
  ExperienceBoost,
  FirstStrike, // temporary or permanent (Dexterous trait)
  FirstStrikeTemporary,
  HeavyFireball,
  // Indulgence
  // Knockback
  Learning,
  // LifeSteal
  MagicalAttack,
  ManaBurned,
  ManaBurnImmune,
  Might,
  // PiercePhysical,
  Poisoned,
  PoisonImmune,
  // Poisonous,
  // Prestige,
  Reflexes,
  Sanguine,
  Schadenfreude,
  SlowStrike,
  SpiritStrength,
  StoneSkin,
  Weakened,
};

constexpr const char* toString(HeroStatus status)
{
  switch (status)
  {
  case HeroStatus::ConsecratedStrike:
    return "Consecrated Strike";
  case HeroStatus::Corrosion:
    return "Corrosion";
  case HeroStatus::CrushingBlow:
    return "Crushing Blow";
  case HeroStatus::Cursed:
    return "Cursed";
  case HeroStatus::CurseImmune:
    return "Curse Immune";
  case HeroStatus::DamageReduction:
    return "Damage Reduction";
  case HeroStatus::DodgePermanent:
    return "Dodge change";
  case HeroStatus::DodgeTemporary:
    return "Dodge change";
  case HeroStatus::DeathProtection:
    return "Death Protection";
  case HeroStatus::ExperienceBoost:
    return "Experience Boost";
  case HeroStatus::FirstStrike:
    return "First Strike";
  case HeroStatus::FirstStrikeTemporary:
    return "First Strike";
  case HeroStatus::HeavyFireball:
    return "Heavy Fireball";
  case HeroStatus::Learning:
    return "Learning";
  case HeroStatus::MagicalAttack:
    return "Magical Attack";
  case HeroStatus::ManaBurned:
    return "Mana Burn";
  case HeroStatus::ManaBurnImmune:
    return "Mana Burn Immune";
  case HeroStatus::Might:
    return "Might";
  case HeroStatus::Poisoned:
    return "Poisoned";
  case HeroStatus::PoisonImmune:
    return "Poison Immune";
  case HeroStatus::Reflexes:
    return "Reflexes";
  case HeroStatus::Sanguine:
    return "Sanguine";
  case HeroStatus::Schadenfreude:
    return "Schadenfreude";
  case HeroStatus::SlowStrike:
    return "Slow Strike";
  case HeroStatus::SpiritStrength:
    return "Spirit Strength";
  case HeroStatus::StoneSkin:
    return "Stone Skin";
  case HeroStatus::Weakened:
    return "Weakened";
  }
}

constexpr bool canHaveMultiple(HeroStatus status)
{
  return status == HeroStatus::Corrosion || status == HeroStatus::CrushingBlow || status == HeroStatus::Cursed ||
         status == HeroStatus::DamageReduction || status == HeroStatus::DodgePermanent ||
         status == HeroStatus::DodgeTemporary || status == HeroStatus::Learning || status == HeroStatus::Sanguine ||
         status == HeroStatus::SpiritStrength || status == HeroStatus::StoneSkin || status == HeroStatus::Weakened;
}
