#include "MonsterTypes.hpp"

MonsterTraits getTraits(MonsterType type)
{
  // Death protection is added in MonsterStats constructor
  // Physical and magical resistances are set in Defence constructor
  MonsterTraitsBuilder traits;
  switch (type)
  {
  case MonsterType::Bandit:
    traits.addCurse();
    break;
  case MonsterType::DragonSpawn:
    traits.addMagicalDamage();
    break;
  case MonsterType::Goat:
    break;
  case MonsterType::Goblin:
    traits.addFirstStrike();
    break;
  case MonsterType::Golem:
    break;
  case MonsterType::GooBlob:
    break;
  case MonsterType::Gorgon:
    traits.addFirstStrike().setDeathGazePercent(50);
    break;
  case MonsterType::MeatMan:
    break;
  case MonsterType::Serpent:
    traits.addPoisonous();
    break;
  case MonsterType::Warlock:
    traits.addMagicalDamage();
    break;
  case MonsterType::Wraith:
    traits.addUndead().addManaBurn().addMagicalDamage();
    break;
  case MonsterType::Zombie:
    traits.addUndead();
    break;
  case MonsterType::AcidBlob:
    traits.addCorrosive().addMagicalDamage();
    break;
  case MonsterType::AnimatedArmour:
    traits.addMagicalDamage();
    break;
  case MonsterType::Berserker:
    traits.setBerserkPercent(50);
    break;
  case MonsterType::BurnViper:
    traits.addManaBurn();
    // traits.addBlinks();
    break;
  case MonsterType::BloodSnake:
    // traits.addSpawns();
    traits.addTODO();
    break;
  case MonsterType::CaveSnake:
    traits.addPoisonous().addTODO();
    // traits.addSpawns();
    break;
  case MonsterType::Changeling:
    // TODO
    traits.addTODO();
    break;
  case MonsterType::Cultist:
    traits.addCowardly();
    // traits.addRevives();
    traits.addTODO();
    break;
  case MonsterType::DesertTroll:
    traits.addCowardly();
    traits.addFastRegen();
    break;
  case MonsterType::Djinn:
    traits.addMagicalDamage().addRetaliate();
    break;
  case MonsterType::DoomArmour:
    traits.setBerserkPercent(50);
    break;
  case MonsterType::Druid:
    traits.addMagicalDamage();
    break;
  case MonsterType::ForestTroll:
    traits.addCowardly();
    traits.addFastRegen();
    break;
  case MonsterType::FrozenTroll:
    traits.addCowardly();
    traits.addMagicalDamage();
    break;
  case MonsterType::GelatinousThing:
    traits.addRetaliate();
    break;
  case MonsterType::Imp:
    // traits.addBlinks();
    break;
  case MonsterType::Illusion:
    traits.addRetaliate().addWeakening();
    break;
  case MonsterType::Minotaur:
    // traits.setKnockbackPercent(50);
    traits.setBerserkPercent(50);
    break;
  case MonsterType::MuckWalker:
    traits.addUndead().addWeakening();
    break;
  case MonsterType::Naga:
    traits.addWeakening();
    break;
  case MonsterType::RockTroll:
    traits.addCowardly();
    // traits.setKnockbackPercent(50);
    traits.addFastRegen();
    break;
  case MonsterType::Rusalka:
    traits.addCorrosive();
    break;
  case MonsterType::SteelGolem:
    traits.addCurse();
    break;
  case MonsterType::Shade:
    // traits.addBlinks();
    traits.addUndead().setLifeStealPercent(40); // TODO: monster life steal not implemented
    break;
  case MonsterType::SlimeBlob:
    traits.addCurse().addMagicalDamage();
    break;
  case MonsterType::Thrall:
    traits.addManaBurn().addPoisonous().addUndead();
    break;
  case MonsterType::Tokoloshe:
    traits.addCowardly();
    // TODO: Drops Tokoloshe Charm
    break;
  case MonsterType::Vampire:
    traits.setLifeStealPercent(40).addMagicalDamage();
    break;
  case MonsterType::Generic:
    break;
  }
  return traits.get();
}
