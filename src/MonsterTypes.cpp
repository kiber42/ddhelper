#include "MonsterTypes.hpp"

MonsterTraits getTraits(MonsterType type)
{
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
  case MonsterType::Warlock:
    traits.addMagicalDamage();
  case MonsterType::Wraith:
    traits.addUndead().addManaBurn().addMagicalDamage();
    break;
  case MonsterType::Zombie:
    traits.addUndead();
    break;
  }
  return traits.get();
}
