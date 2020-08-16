#pragma once

//#include "HeroModifier.h"
//#include "Inventory.h"

#include "Attack.hpp"
#include "Defence.hpp"
#include "ExperienceInterface.hpp"
#include "HeroStats.hpp"
#include "HeroStatus.hpp"

#include <map>
#include <memory>

class AttackBehaviour;
class Monster;

class Hero
{
public:
  Hero();
  Hero(const Hero &);
  Hero(Hero &&);

  int getXP() const;
  int getLevel() const;
  int getPrestige() const;
  void gainExperience(int xpGained);
  void gainLevel();
  void modifyLevelBy(int delta);

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;
  int getManaPoints() const;
  int getManaPointsMax() const;
  void modifyHitPointsMax(int delta);

  int getBaseDamage() const;
  void changeBaseDamage(int deltaDamagePoints);
  int getDamageBonusPercent() const;
  void changeDamageBonusPercent(int deltaDamageBonusPercent);
  int getDamage() const;

  bool hasInitiativeVersus(const Monster &monster) const;
  int predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const;
  void takeDamage(int attackerDamageOutput, bool isMagicalDamage);

  void healHitPoints(int amountPointsHealed, bool mayOverheal = false);
  void loseHitPointsOutsideOfFight(int amountPointsLost);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);

  int getDeathProtection() const;
  void setDeathProtection(bool enableProtection);

  void addStatus(HeroStatus status, int addedIntensity = 1);
  void removeStatus(HeroStatus status, bool completely = true);
  bool hasStatus(HeroStatus status) const;
  void setStatusIntensity(HeroStatus status, int newIntensity);
  int getStatusIntensity(HeroStatus status) const;

private:
  HeroStats stats;
  std::unique_ptr<AttackBehaviour> attack;
  Defence defence;
  std::unique_ptr<ExperienceInterface> experience;
  std::map<HeroStatus, int> statuses;

  void statusAddedHook(HeroStatus status);
  void propagateStatus(HeroStatus status);
  void levelGainedUpdate();

  /*
   * 	int piety;
          int gold;
          int conversionPoints;

          std::unique_ptr<Inventory> inventory;
          std::vector<std::unique_ptr<HeroModifier>> modifiers;
          */
};
