#include "MonsterTraits.hpp"

#include "MonsterTypes.hpp"

#include <utility>

MonsterTraits::MonsterTraits(MonsterType type)
  : MonsterTraits(getTraits(type))
{
}

MonsterTraits::MonsterTraits(MonsterTraitsBuilder& builder)
  : MonsterTraits(builder.get())
{
}

MonsterTraits&& MonsterTraitsBuilder::get()
{
  return std::move(traits);
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

MonsterTraitsBuilder& MonsterTraitsBuilder::setBerserkPercent(int newBerserkPercent)
{
  traits.berserkPercent = newBerserkPercent;
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

MonsterTraitsBuilder& MonsterTraitsBuilder::addFastRegen()
{
  traits.fastRegen = true;
  return *this;
}
