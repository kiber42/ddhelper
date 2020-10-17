#pragma once

#include <iostream>

enum class MonsterType;
class MonsterTraitsBuilder;

struct MonsterTraits
{
  bool doesMagicalDamage() const
  {
    return magicalDamage;
  }
  bool doesRetaliate() const
  {
    return retaliate;
  }

  bool isPoisonous() const
  {
    return poisonous;
  }
  bool hasManaBurn() const
  {
    return manaBurn;
  }
  bool bearsCurse() const
  {
    return curse;
  }
  bool isCorrosive() const
  {
    return corrosive;
  }
  bool isWeakening() const
  {
    return weakening;
  }

  bool hasFirstStrike() const
  {
    return firstStrike;
  }
  int getDeathGazePercent() const
  {
    return deathGazePercent;
  }
  int getLifeStealPercent() const
  {
    return lifeStealPercent;
  }
  int getBerserkPercent() const
  {
    return berserkPercent;
  }
  bool isUndead() const
  {
    return undead;
  }
  bool isBloodless() const
  {
    return bloodless;
  }
  bool hasFastRegen() const
  {
    return fastRegen;
  }

  bool magicalDamage;
  bool retaliate;
  bool poisonous;
  bool manaBurn;
  bool curse;
  bool corrosive;
  bool weakening;
  bool firstStrike;
  int deathGazePercent;
  int lifeStealPercent;
  int berserkPercent;
  bool undead;
  bool bloodless;
  bool fastRegen;

  MonsterTraits();
  MonsterTraits(MonsterType type);
  MonsterTraits(MonsterTraitsBuilder&& builder);
};

class MonsterTraitsBuilder
{
public:
  MonsterTraitsBuilder() = default;
  MonsterTraits get();

  MonsterTraitsBuilder& addTODO()
  {
    std::cerr << "Important monster trait not implemented." << std::endl;
    return *this;
  }

  MonsterTraitsBuilder& addMagicalDamage();
  MonsterTraitsBuilder& addRetaliate();
  MonsterTraitsBuilder& addPoisonous();
  MonsterTraitsBuilder& addManaBurn();
  MonsterTraitsBuilder& addCurse();
  MonsterTraitsBuilder& addCorrosive();
  MonsterTraitsBuilder& addWeakening();
  MonsterTraitsBuilder& addFirstStrike();
  MonsterTraitsBuilder& setDeathGazePercent(int deathGazePercent);
  MonsterTraitsBuilder& setLifeStealPercent(int lifeStealPercent);
  MonsterTraitsBuilder& setBerserkPercent(int newBerserkPercent);
  MonsterTraitsBuilder& addUndead();
  MonsterTraitsBuilder& addBloodless();
  MonsterTraitsBuilder& addFastRegen();

private:
  MonsterTraits traits;
};
