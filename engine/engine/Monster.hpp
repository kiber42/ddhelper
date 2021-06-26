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
  Monster(MonsterType type, uint8_t level, uint8_t dungeonMultiplier = 100);
  Monster(std::string name, MonsterStats, Defence, MonsterTraits);
  Monster(uint8_t level, uint16_t hp, uint16_t damage);

  const std::string& getName() const;
  int getID() const;
  unsigned getLevel() const;
  bool isDefeated() const;

  unsigned getHitPoints() const;
  unsigned getHitPointsMax() const;
  unsigned getDamage() const;

  unsigned getPhysicalResistPercent() const;
  unsigned getMagicalResistPercent() const;

  unsigned predictDamageTaken(unsigned attackerDamageOutput, DamageType damageType) const;
  void takeDamage(unsigned attackerDamageOutput, DamageType damageType);
  void takeFireballDamage(unsigned casterLevel, unsigned damageMultiplier = 4);
  void takeBurningStrikeDamage(unsigned attackerDamageOutput, unsigned casterLevel, DamageType damageType);
  void takeManaShieldDamage(unsigned casterLevel);
  void receiveCrushingBlow();
  void recover(unsigned nSquares);
  void burn(unsigned nMaxStacks);
  void burnMax(unsigned nMaxStacks);
  void burnDown();
  // returns false for undead monsters as they cannot be poisoned
  bool poison(unsigned addedPoisonAmount);
  void slow();
  void erodeResitances();
  void petrify();
  void die();
  void corrode(unsigned amount = 1);
  void zot();
  void makeWickedSick();

  bool isBurning() const;
  bool isPoisoned() const;
  bool isSlowed() const;
  bool isZotted() const;
  bool isWickedSick() const;

  unsigned getBurnStackSize() const;
  unsigned getPoisonAmount() const;
  unsigned getDeathProtection() const;
  unsigned getCorroded() const;

  bool has(MonsterTrait trait) const;
  unsigned getDeathGazePercent() const;
  unsigned getLifeStealPercent() const;

  DamageType damageType() const;

  // Boosts from punishments
  void changePhysicalResist(int deltaPercent);
  void changeMagicResist(int deltaPercent);
  void applyTikkiTookiBoost();

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
