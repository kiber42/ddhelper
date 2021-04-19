#pragma once

#include "engine/Defence.hpp"
#include "engine/MonsterStats.hpp"
#include "engine/MonsterStatus.hpp"
#include "engine/MonsterTraits.hpp"

#include <string>
#include <vector>

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

  int predictDamageTaken(int attackerDamageOutput, DamageType damageType) const;
  void takeDamage(int attackerDamageOutput, DamageType damageType);
  void takeFireballDamage(int casterLevel, int damageMultiplier = 4);
  void takeBurningStrikeDamage(int attackerDamageOutput, int casterLevel, DamageType damageType);
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
  void zot();
  void makeWickedSick();

  bool isBurning() const;
  bool isPoisoned() const;
  bool isSlowed() const;
  bool isZotted() const;
  bool isWickedSick() const;

  int getBurnStackSize() const;
  int getPoisonAmount() const;
  int getDeathProtection() const;
  int getCorroded() const;

  bool doesMagicalDamage() const;
  DamageType damageType() const;
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
  bool isCowardly() const;

  // Boosts from punishments
  void addPhysicalResist(int additionalResistPercent);
  void addMagicResist(int additionalResistPercent);
  void makeFast();
  void makeWeakening();

  // TODO: Add non xp-valuable monsters, and plants
  bool grantsXP() const { return true; }

private:
  std::string name;
  int id;

  MonsterStats stats;
  Defence defence;
  MonsterStatus status;
  MonsterTraits traits;

  static int lastId;
};

using Monsters = std::vector<Monster>;

std::vector<std::string> describe(const Monster& monster);

inline bool operator==(const Monster& lhs, const Monster& rhs)
{
  return lhs.getID() == rhs.getID();
}

inline bool operator!=(const Monster& lhs, const Monster& rhs)
{
  return !(lhs == rhs);
}
