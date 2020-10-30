#pragma once

#include "Defence.hpp"
#include "MonsterStats.hpp"
#include "MonsterStatus.hpp"
#include "MonsterTraits.hpp"

#include <string>

class Monster
{
public:
  Monster(MonsterType type, int level, int dungeonMultiplier = 100);
  Monster(std::string name, MonsterStats, Defence, MonsterTraits);
  Monster(int level, int hp, int damage);

  const char* getName() const;
  int getID() const;
  int getLevel() const;
  bool isDefeated() const;

  int getHitPoints() const;
  int getHitPointsMax() const;
  int getDamage() const;

  int getPhysicalResistPercent() const;
  int getMagicalResistPercent() const;

  void takeDamage(int attackerDamageOutput, bool isMagicalDamage);
  void takeFireballDamage(int casterLevel, int damageMultiplier = 4);
  void takeManaShieldDamage(int casterLevel);
  void receiveCrushingBlow();
  void recover(int nSquares);
  void burn(int nMaxStacks);
  void burnMax(int nMaxStacks);
  void burnDown();
  // returns false for undead monsters as they cannot be poisoned
  bool poison(int addedPoisonAmount);
  void slow();
  void erodeResitances();
  void petrify();
  void die();
  void corrode(int amount = 1);

  bool isBurning() const;
  bool isPoisoned() const;
  bool isSlowed() const;
  int getBurnStackSize() const;
  int getPoisonAmount() const;
  int getDeathProtection() const;
  int getCorroded() const;

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

  // TODO: Add non xp-valuable monsters, and plants
  bool grantsXP() const { return true; }

private:
  int predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const;

  std::string name;
  int id;

  MonsterStats stats;
  Defence defence;
  MonsterStatus status;
  MonsterTraits traits;

  static int lastId;
};
