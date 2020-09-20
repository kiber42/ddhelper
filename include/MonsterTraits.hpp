#pragma once

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
  bool isUndead() const
  {
    return undead;
  }
  bool isBloodless() const
  {
    return bloodless;
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
  bool undead;
  bool bloodless;

  MonsterTraits();
  MonsterTraits(MonsterType type);
  MonsterTraits(MonsterTraitsBuilder&& builder);
};

class MonsterTraitsBuilder
{
public:
  MonsterTraitsBuilder() = default;
  MonsterTraits get();

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
  MonsterTraitsBuilder& addUndead();
  MonsterTraitsBuilder& addBloodless();
  MonsterTraitsBuilder& removeMagicalDamage();
  MonsterTraitsBuilder& removeRetaliate();
  MonsterTraitsBuilder& removePoisonous();
  MonsterTraitsBuilder& removeManaBurn();
  MonsterTraitsBuilder& removeCurse();
  MonsterTraitsBuilder& removeCorrosive();
  MonsterTraitsBuilder& removeWeakening();
  MonsterTraitsBuilder& removeFirstStrike();
  MonsterTraitsBuilder& removeUndead();
  MonsterTraitsBuilder& removeBloodless();

private:
  MonsterTraits traits;
};
