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

  bool magicalDamage{false};
  bool retaliate{false};
  bool poisonous{false};
  bool manaBurn{false};
  bool curse{false};
  bool corrosive{false};
  bool weakening{false};
  bool firstStrike{false};
  int deathGazePercent{0};
  int lifeStealPercent{0};
  int berserkPercent{0};
  bool undead{false};
  bool bloodless{false};
  bool fastRegen{false};

  MonsterTraits() = default;
  MonsterTraits(MonsterType type);
  MonsterTraits(MonsterTraitsBuilder& builder);
};

class MonsterTraitsBuilder
{
public:
  MonsterTraits&& get();

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
