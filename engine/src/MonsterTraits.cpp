#include "engine/MonsterTraits.hpp"

MonsterTraits::MonsterTraits(MonsterType type)
{
  // Death protection is added in MonsterStats constructor
  // Physical and magical resistances are set in Defence constructor
  switch (type)
  {
  case MonsterType::Bandit:
    add(MonsterTrait::CurseBearer);
    break;
  case MonsterType::DragonSpawn:
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::Goblin:
    add(MonsterTrait::FirstStrike);
    break;
  case MonsterType::Gorgon:
    add(MonsterTrait::FirstStrike);
    deathGaze_ = 50_deathgaze;
    break;
  case MonsterType::Serpent:
    add(MonsterTrait::Poisonous);
    break;
  case MonsterType::Warlock:
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::Wraith:
    add(MonsterTrait::Bloodless);
    add(MonsterTrait::Undead);
    add(MonsterTrait::ManaBurn);
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::Zombie:
    add(MonsterTrait::Undead);
    break;
  case MonsterType::AcidBlob:
    add(MonsterTrait::Corrosive);
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::AnimatedArmour:
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::Berserker:
    berserk_ = 50_berserk;
    break;
  case MonsterType::BurnViper:
    add(MonsterTrait::ManaBurn);
    add(MonsterTrait::Blinks);
    break;
  case MonsterType::BloodSnake:
    add(MonsterTrait::Spawns);
    break;
  case MonsterType::CaveSnake:
    add(MonsterTrait::Poisonous);
    add(MonsterTrait::Spawns);
    break;
  case MonsterType::Changeling:
    // TODO
    break;
  case MonsterType::Cultist:
    add(MonsterTrait::Cowardly);
    add(MonsterTrait::Revives);
    break;
  case MonsterType::DesertTroll:
    add(MonsterTrait::Cowardly);
    add(MonsterTrait::FastRegen);
    break;
  case MonsterType::Djinn:
    add(MonsterTrait::MagicalAttack);
    add(MonsterTrait::Retaliate);
    break;
  case MonsterType::DoomArmour:
    berserk_ = 50_berserk;
    break;
  case MonsterType::Druid:
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::ForestTroll:
    add(MonsterTrait::Cowardly);
    add(MonsterTrait::FastRegen);
    break;
  case MonsterType::FrozenTroll:
    add(MonsterTrait::Cowardly);
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::GelatinousThing:
    add(MonsterTrait::Retaliate);
    break;
  case MonsterType::Imp:
    add(MonsterTrait::Blinks);
    break;
  case MonsterType::Illusion:
    add(MonsterTrait::Retaliate);
    add(MonsterTrait::Weakening);
    break;
  case MonsterType::Minotaur:
    berserk_ = 50_berserk;
    knockback_ = 50_knockback;
    break;
  case MonsterType::MuckWalker:
    add(MonsterTrait::Bloodless);
    add(MonsterTrait::Undead);
    add(MonsterTrait::Weakening);
    break;
  case MonsterType::Naga:
    add(MonsterTrait::Weakening);
    break;
  case MonsterType::RockTroll:
    add(MonsterTrait::Cowardly);
    add(MonsterTrait::FastRegen);
    knockback_ = 50_knockback;
    break;
  case MonsterType::Rusalka:
    add(MonsterTrait::Corrosive);
    break;
  case MonsterType::SteelGolem:
    add(MonsterTrait::CurseBearer);
    break;
  case MonsterType::Shade:
    add(MonsterTrait::Blinks);
    add(MonsterTrait::Undead);
    lifeSteal_ = 40_lifesteal;
    break;
  case MonsterType::SlimeBlob:
    add(MonsterTrait::CurseBearer);
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::Thrall:
    add(MonsterTrait::Bloodless);
    add(MonsterTrait::ManaBurn);
    add(MonsterTrait::Poisonous);
    add(MonsterTrait::Undead);
    break;
  case MonsterType::Tokoloshe:
    // TODO: Drops Tokoloshe Charm
    add(MonsterTrait::Cowardly);
    break;
  case MonsterType::Vampire:
    lifeSteal_ = 40_lifesteal;
    add(MonsterTrait::MagicalAttack);
    break;
  case MonsterType::Ratling:
    add(MonsterTrait::Retaliate);
    add(MonsterTrait::FastRegen);
    break;
  case MonsterType::Succubus:
    deathGaze_ = 50_deathgaze;
    add(MonsterTrait::Weakening);
    break;
  case MonsterType::Goat:
  case MonsterType::Golem:
  case MonsterType::GooBlob:
  case MonsterType::MeatMan:
  case MonsterType::Generic:
    break;
  }
}

MonsterTraits::MonsterTraits(std::initializer_list<MonsterTrait> traits)
{
  for (auto trait : traits)
    add(trait);
}
