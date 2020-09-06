#pragma once

//#include "Inventory.h"

#include "Defence.hpp"
#include "Experience.hpp"
#include "HeroStats.hpp"
#include "HeroStatus.hpp"
#include "HeroTraits.hpp"

#include <map>
#include <memory>
#include <set>

class Monster;

class Hero
{
public:
  Hero();
  Hero(HeroStats, Defence, Experience);

  int getXP() const;
  int getLevel() const;
  int getPrestige() const;
  int getXPforNextLevel() const;
  void gainExperience(int xpGained, bool monsterWasSlowed=false);
  void gainLevel();
  void modifyLevelBy(int delta);

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;
  int getManaPoints() const;
  int getManaPointsMax() const;

  int getBaseDamage() const;
  void changeBaseDamage(int deltaDamagePoints);
  int getDamageBonusPercent() const;
  void changeDamageBonusPercent(int deltaDamageBonusPercent);
  int getDamage() const;

  int getPhysicalResistPercent() const;
  int getMagicalResistPercent() const;
  void setPhysicalResistPercent(int physicalResistPercent);
  void setMagicalResistPercent(int magicalResistPercent);
  void changePhysicalResistPercent(int deltaPercent);
  void changeMagicalResistPercent(int deltaPercent);

  bool doesMagicalDamage() const;

  bool hasInitiativeVersus(const Monster &monster) const;
  int predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const;
  void takeDamage(int attackerDamageOutput, bool isMagicalDamage);
  void recover(int nSquares);

  void healHitPoints(int amountPointsHealed, bool mayOverheal = false);
  void loseHitPointsOutsideOfFight(int amountPointsLost);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);

  int getDeathProtection() const;
  void setDeathProtection(bool enableProtection);

  void addStatus(HeroStatus status, int addedIntensity = 1);
  void removeStatus(HeroStatus status, bool completely);
  bool hasStatus(HeroStatus status) const;
  void setStatusIntensity(HeroStatus status, int newIntensity);
  int getStatusIntensity(HeroStatus status) const;

  void addTrait(HeroTrait trait);
  bool hasTrait(HeroTrait trait) const;

private:
  HeroStats stats;
  Defence defence;
  Experience experience;
  std::map<HeroStatus, int> statuses;
  std::set<HeroTrait> traits;

  void propagateStatus(HeroStatus status, int intensity);
  void levelGainedUpdate();

  /*
   * 	int piety;
          int gold;
          int conversionPoints;

          std::unique_ptr<Inventory> inventory;
          */
};
