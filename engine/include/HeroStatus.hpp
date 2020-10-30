#pragma once

enum class HeroStatus
{
  BurningStrike, // TODO
  ConsecratedStrike,
  Corrosion,
  CorrosiveStrike, // TODO
  CrushingBlow,
  Cursed,
  CurseImmune,
  DamageReduction,
  DeathGaze, // TODO
  DeathGazeImmune,
  DeathProtection,
  DodgePermanent,
  DodgeTemporary,
  Exhausted,
  ExperienceBoost,
  FirstStrike, // temporary or permanent (Dexterous trait)
  FirstStrikeTemporary,
  HeavyFireball,
  Knockback, // TODO
  Learning,
  LifeSteal,
  MagicalAttack,
  ManaBurned,
  ManaBurnImmune,
  Might,
  PiercePhysical, // TODO
  Poisoned,
  Poisonous,
  PoisonImmune,
  Reflexes,
  Sanguine,
  Schadenfreude,
  SlowStrike,
  SpiritStrength,
  StoneSkin,
  Weakened,
  Last = Weakened,
};

constexpr const char* toString(HeroStatus status)
{
  switch (status)
  {
  case HeroStatus::BurningStrike:
    return "Burning Strike";
  case HeroStatus::ConsecratedStrike:
    return "Consecrated Strike";
  case HeroStatus::Corrosion:
    return "Corrosion";
  case HeroStatus::CorrosiveStrike:
    return "Corrosive Strike";
  case HeroStatus::CrushingBlow:
    return "Crushing Blow";
  case HeroStatus::Cursed:
    return "Cursed";
  case HeroStatus::CurseImmune:
    return "Curse Immune";
  case HeroStatus::DamageReduction:
    return "Damage Reduction";
  case HeroStatus::DeathGaze:
    return "Death Gaze";
  case HeroStatus::DeathGazeImmune:
    return "Death Gaze Immune";
  case HeroStatus::DeathProtection:
    return "Death Protection";
  case HeroStatus::DodgePermanent:
    return "Permanent Dodge Chance";
  case HeroStatus::PiercePhysical:
    return "Pierce Physical";
  case HeroStatus::DodgeTemporary:
    return "Dodge Chance";
  case HeroStatus::Exhausted:
    return "Exhausted";
  case HeroStatus::ExperienceBoost:
    return "Experience Boost";
  case HeroStatus::FirstStrike:
    return "First Strike";
  case HeroStatus::FirstStrikeTemporary:
    return "First Strike";
  case HeroStatus::HeavyFireball:
    return "Heavy Fireball";
  case HeroStatus::Knockback:
    return "Knockback";
  case HeroStatus::Learning:
    return "Learning";
  case HeroStatus::LifeSteal:
    return "Life Steal";
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
  case HeroStatus::Poisonous:
    return "Poisonous";
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
  return status == HeroStatus::Corrosion || status == HeroStatus::CorrosiveStrike ||
         status == HeroStatus::CrushingBlow || status == HeroStatus::Cursed || status == HeroStatus::DamageReduction ||
         status == HeroStatus::DeathGaze || status == HeroStatus::DodgePermanent ||
         status == HeroStatus::DodgeTemporary || status == HeroStatus::Knockback || status == HeroStatus::Learning ||
         status == HeroStatus::LifeSteal || status == HeroStatus::Poisonous || status == HeroStatus::Sanguine ||
         status == HeroStatus::SpiritStrength || status == HeroStatus::StoneSkin || status == HeroStatus::Weakened;
}
