#include "engine/MonsterTypes.hpp"

MonsterTraits getTraits(MonsterType type)
{
  // Death protection is added in MonsterStats constructor
  // Physical and magical resistances are set in Defence constructor
  switch (type)
  {
  case MonsterType::Bandit:
    return MonsterTraitsBuilder().addCurse();
  case MonsterType::DragonSpawn:
    return MonsterTraitsBuilder().addMagicalDamage();
  case MonsterType::Goat:
    return {};
  case MonsterType::Goblin:
    return MonsterTraitsBuilder().addFirstStrike();
  case MonsterType::Golem:
    return {};
  case MonsterType::GooBlob:
    return {};
  case MonsterType::Gorgon:
    return MonsterTraitsBuilder().addFirstStrike().setDeathGazePercent(50);
  case MonsterType::MeatMan:
    return {};
  case MonsterType::Serpent:
    return MonsterTraitsBuilder().addPoisonous();
  case MonsterType::Warlock:
    return MonsterTraitsBuilder().addMagicalDamage();
  case MonsterType::Wraith:
    return MonsterTraitsBuilder().addUndead().addManaBurn().addMagicalDamage();
  case MonsterType::Zombie:
    return MonsterTraitsBuilder().addUndead();
  case MonsterType::AcidBlob:
    return MonsterTraitsBuilder().addCorrosive().addMagicalDamage();
  case MonsterType::AnimatedArmour:
    return MonsterTraitsBuilder().addMagicalDamage();
  case MonsterType::Berserker:
    return MonsterTraitsBuilder().setBerserkPercent(50);
  case MonsterType::BurnViper:
    return MonsterTraitsBuilder().addManaBurn().addBlinks();
  case MonsterType::BloodSnake:
    return MonsterTraitsBuilder().addSpawns();
  case MonsterType::CaveSnake:
    return MonsterTraitsBuilder().addPoisonous().addSpawns();
  case MonsterType::Changeling:
    // TODO
    return {};
  case MonsterType::Cultist:
    return MonsterTraitsBuilder().addCowardly().addRevives();
  case MonsterType::DesertTroll:
    return MonsterTraitsBuilder().addCowardly().addFastRegen();
  case MonsterType::Djinn:
    return MonsterTraitsBuilder().addMagicalDamage().addRetaliate();
  case MonsterType::DoomArmour:
    return MonsterTraitsBuilder().setBerserkPercent(50);
  case MonsterType::Druid:
    return MonsterTraitsBuilder().addMagicalDamage();
  case MonsterType::ForestTroll:
    return MonsterTraitsBuilder().addCowardly().addFastRegen();
  case MonsterType::FrozenTroll:
    return MonsterTraitsBuilder().addCowardly().addMagicalDamage();
  case MonsterType::GelatinousThing:
    return MonsterTraitsBuilder().addRetaliate();
  case MonsterType::Imp:
    return MonsterTraitsBuilder().addBlinks();
  case MonsterType::Illusion:
    return MonsterTraitsBuilder().addRetaliate().addWeakening();
  case MonsterType::Minotaur:
    return MonsterTraitsBuilder().setBerserkPercent(50).setKnockbackPercent(50);
  case MonsterType::MuckWalker:
    return MonsterTraitsBuilder().addUndead().addWeakening();
  case MonsterType::Naga:
    return MonsterTraitsBuilder().addWeakening();
  case MonsterType::RockTroll:
    return MonsterTraitsBuilder().addCowardly().addFastRegen().setKnockbackPercent(50);
  case MonsterType::Rusalka:
    return MonsterTraitsBuilder().addCorrosive();
  case MonsterType::SteelGolem:
    return MonsterTraitsBuilder().addCurse();
  case MonsterType::Shade:
    return MonsterTraitsBuilder().addUndead().setLifeStealPercent(40).addBlinks();
  case MonsterType::SlimeBlob:
    return MonsterTraitsBuilder().addCurse().addMagicalDamage();
  case MonsterType::Thrall:
    return MonsterTraitsBuilder().addManaBurn().addPoisonous().addUndead();
  case MonsterType::Tokoloshe:
    // TODO: Drops Tokoloshe Charm
    return MonsterTraitsBuilder().addCowardly();
  case MonsterType::Vampire:
    return MonsterTraitsBuilder().setLifeStealPercent(40).addMagicalDamage();
  case MonsterType::Generic:
    return {};
  }
}
