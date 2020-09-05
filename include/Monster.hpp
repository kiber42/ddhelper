#pragma once

#include "Defence.hpp"
#include "MonsterStats.hpp"
#include "MonsterStatus.hpp"
#include "MonsterTraits.hpp"

class Monster
{
public:
  Monster(MonsterStats, Defence, MonsterTraits);

  int getLevel() const;
  bool isDefeated() const;

  int getHitPoints() const;
  int getHitPointsMax() const;
  int getDamage() const;

  int getPhysicalResistPercent() const;
  int getMagicalResistPercent() const;

  void takeDamage(int attackerDamageOutput, bool isMagicalDamage);
  void takeFireballDamage(int casterLevel, int damageMultiplier = 4);
  void recover(int nSquares);
  void burn(int nMaxStacks);
  void burnMax(int nMaxStacks);
  void burnDown();
  void poison(int addedPoisonAmount);
  void stun();
  void erodeResitances();
  void petrify();

  bool isBurning() const;
  bool isPoisoned() const;
  bool isStunned() const;
  int getBurnStackSize() const;
  int getPoisonAmount() const;
  int getDeathProtection() const;
  int getCorroded() const;
  int getWeakened() const;

  void corrode();
  void weaken();

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
  int predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const;

  MonsterStats stats;
  Defence defence;
  MonsterStatus status;
  MonsterTraits traits;
};
