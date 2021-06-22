#pragma once

enum class HeroStatus
{
  BurningStrike,
  ConsecratedStrike,
  CorrosiveStrike,
  CrushingBlow,
  CurseImmune,
  DamageReduction,
  DeathGaze, // TODO
  DeathGazeImmune,
  DeathProtection,
  DodgePermanent,
  DodgeTemporary,
  DodgePrediction,
  Exhausted,
  ExperienceBoost,
  FirstStrikePermanent,
  FirstStrikeTemporary,
  Healthform,
  HeavyFireball,
  Knockback,
  Learning,
  LifeSteal,
  MagicalAttack,
  ManaBurnImmune,
  Manaform,
  Might,
  Momentum,
  PiercePhysical,
  Poisonous,
  PoisonImmune,
  Reflexes,
  Sanguine,
  Schadenfreude,
  SlowStrike,
  SpiritStrength,
  StoneSkin,
  Last = StoneSkin,
  Pessimist,     // Loses all dice rolls
  ByssepsStacks, // Used for Chemist
};

enum class HeroDebuff
{
  Corroded,
  Cursed,
  ManaBurned,
  Poisoned,
  Weakened,
  Last = Weakened
};

constexpr const char* toString(HeroStatus status)
{
  switch (status)
  {
  case HeroStatus::BurningStrike:
    return "Burning Strike";
  case HeroStatus::ConsecratedStrike:
    return "Consecrated Strike";
  case HeroStatus::CorrosiveStrike:
    return "Corrosive Strike";
  case HeroStatus::CrushingBlow:
    return "Crushing Blow";
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
  case HeroStatus::DodgePrediction:
    return "Dodge Prediction";
  case HeroStatus::DodgeTemporary:
    return "Dodge Chance";
  case HeroStatus::Exhausted:
    return "Exhausted";
  case HeroStatus::ExperienceBoost:
    return "Experience Boost";
  case HeroStatus::FirstStrikePermanent:
    return "First Strike (permanent)";
  case HeroStatus::FirstStrikeTemporary:
    return "First Strike (next attack)";
  case HeroStatus::Healthform:
    return "Healthform";
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
  case HeroStatus::ManaBurnImmune:
    return "Mana Burn Immune";
  case HeroStatus::Manaform:
    return "Manaform";
  case HeroStatus::Might:
    return "Might";
  case HeroStatus::Momentum:
    return "Momentum";
  case HeroStatus::PiercePhysical:
    return "Pierce Physical";
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
  case HeroStatus::Pessimist:
    return "Pessimist";
  case HeroStatus::ByssepsStacks:
    return "Bysseps";
  }
}

constexpr const char* toString(HeroDebuff debuff)
{
  switch (debuff)
  {
  case HeroDebuff::Corroded:
    return "Corroded";
  case HeroDebuff::Cursed:
    return "Cursed";
  case HeroDebuff::ManaBurned:
    return "Mana Burned";
  case HeroDebuff::Poisoned:
    return "Poisoned";
  case HeroDebuff::Weakened:
    return "Weakened";
  }
}

constexpr bool canHaveMultiple(HeroStatus status)
{
  return status == HeroStatus::ByssepsStacks || status == HeroStatus::CorrosiveStrike ||
         status == HeroStatus::CrushingBlow || status == HeroStatus::DamageReduction ||
         status == HeroStatus::DeathGaze || status == HeroStatus::DodgePermanent ||
         status == HeroStatus::DodgeTemporary || status == HeroStatus::Knockback || status == HeroStatus::Learning ||
         status == HeroStatus::LifeSteal || status == HeroStatus::Might || status == HeroStatus::Momentum ||
         status == HeroStatus::Poisonous || status == HeroStatus::Sanguine || status == HeroStatus::SpiritStrength ||
         status == HeroStatus::StoneSkin;
}

constexpr bool canHaveMultiple(HeroDebuff debuff)
{
  return debuff == HeroDebuff::Corroded || debuff == HeroDebuff::Cursed || debuff == HeroDebuff::Weakened;
}
