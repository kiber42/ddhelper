#include "MonsterTraits.hpp"

#include "MonsterTypes.hpp"

#include <utility>

MonsterTraits::MonsterTraits()
  : magicalDamage(false)
  , retaliate(false)
  , poisonous(false)
  , manaBurn(false)
  , curse(false)
  , corrosive(false)
  , weakening(false)
  , firstStrike(false)
  , deathGazePercent(0)
  , lifeStealPercent(0)
  , undead(false)
  , bloodless(false)
{
}

MonsterTraits::MonsterTraits(MonsterType type)
  : MonsterTraits(getTraits(type))
{
}

MonsterTraits::MonsterTraits(MonsterTraitsBuilder&& builder)
  : MonsterTraits(std::move(builder.get()))
{
}

MonsterTraits MonsterTraitsBuilder::get()
{
  return traits;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addMagicalDamage()
{
  traits.magicalDamage = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addRetaliate()
{
  traits.retaliate = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addPoisonous()
{
  traits.poisonous = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addManaBurn()
{
  traits.manaBurn = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addCurse()
{
  traits.curse = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addCorrosive()
{
  traits.corrosive = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addWeakening()
{
  traits.weakening = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addFirstStrike()
{
  traits.firstStrike = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::setDeathGazePercent(int newDeathGazePercent)
{
  traits.deathGazePercent = newDeathGazePercent;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::setLifeStealPercent(int newLifeStealPercent)
{
  traits.lifeStealPercent = newLifeStealPercent;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addUndead()
{
  traits.undead = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::addBloodless()
{
  traits.bloodless = true;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeMagicalDamage()
{
  traits.magicalDamage = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeRetaliate()
{
  traits.retaliate = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removePoisonous()
{
  traits.poisonous = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeManaBurn()
{
  traits.manaBurn = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeCurse()
{
  traits.curse = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeCorrosive()
{
  traits.corrosive = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeWeakening()
{
  traits.weakening = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeFirstStrike()
{
  traits.firstStrike = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeUndead()
{
  traits.undead = false;
  return *this;
}

MonsterTraitsBuilder& MonsterTraitsBuilder::removeBloodless()
{
  traits.bloodless = false;
  return *this;
}
