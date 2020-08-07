#pragma once

class MonsterTraits
{
public:
  MonsterTraits();
  MonsterTraits &addMagicalDamage();
  MonsterTraits &addRetaliate();
  MonsterTraits &addPoisonous();
  MonsterTraits &addManaBurn();
  MonsterTraits &addCurse();
  MonsterTraits &addCorrosive();
  MonsterTraits &addWeakening();
  MonsterTraits &addFirstStrike();
  MonsterTraits &setDeathGazePercent(int deathGazePercent);
  MonsterTraits &setLifeStealPercent(int lifeStealPercent);
  MonsterTraits &addUndead();
  MonsterTraits &addBloodless();
  MonsterTraits &removeMagicalDamage();
  MonsterTraits &removeRetaliate();
  MonsterTraits &removePoisonous();
  MonsterTraits &removeManaBurn();
  MonsterTraits &removeCurse();
  MonsterTraits &removeCorrosive();
  MonsterTraits &removeWeakening();
  MonsterTraits &removeFirstStrike();
  MonsterTraits &removeUndead();
  MonsterTraits &removeBloodless();

  bool doesMagicalDamage() const;
  bool doesRetaliate() const;

  bool isPoisonous() const;
  bool hasManaBurn() const;
  bool bearsCurse() const;
  bool isCorrosive() const;
  bool isWeakening() const;

  bool hasFirstStrike() const;
  int getDeathGazePercent() const;
  int getLifeStealPercent() const;
  bool isUndead() const;
  bool isBloodless() const;

private:
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
};
