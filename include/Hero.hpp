#pragma once

#include "Conversion.hpp"
#include "Defence.hpp"
#include "Experience.hpp"
#include "Faith.hpp"
#include "HeroClass.hpp"
#include "HeroStats.hpp"
#include "HeroStatus.hpp"
#include "HeroTraits.hpp"
#include "Inventory.hpp"

#include <map>
#include <string>
#include <vector>

class Monster;

class Hero
{
public:
  Hero();
  Hero(HeroClass, HeroRace);
  Hero(HeroStats, Defence, Experience);

  std::string getName() const;

  int getXP() const;
  int getLevel() const;
  int getPrestige() const;
  int getXPforNextLevel() const;
  void gainExperience(int xpGained, bool monsterWasSlowed = false);
  void gainLevel();

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;
  int getManaPoints() const;
  int getManaPointsMax() const;

  void drinkHealthPotion();
  void drinkManaPotion();

  int getBaseDamage() const;
  void changeBaseDamage(int deltaDamagePoints);
  int getDamageBonusPercent() const;
  void changeDamageBonusPercent(int deltaDamageBonusPercent);
  int getDamageVersusStandard() const;
  int getDamageVersus(const Monster& monster) const;

  void addHealthBonus();
  void addManaBonus();
  void addDamageBonus();

  int getPhysicalResistPercent() const;
  int getMagicalResistPercent() const;
  void setPhysicalResistPercent(int physicalResistPercent);
  void setMagicalResistPercent(int magicalResistPercent);
  void changePhysicalResistPercent(int deltaPercent);
  void changeMagicalResistPercent(int deltaPercent);

  bool doesMagicalDamage() const;

  bool hasInitiativeVersus(const Monster& monster) const;
  int predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const;
  void takeDamage(int attackerDamageOutput, bool isMagicalDamage);
  void recover(int nSquares);
  int numSquaresForFullRecovery() const;

  void healHitPoints(int amountPointsHealed, bool mayOverheal = false);
  void loseHitPointsOutsideOfFight(int amountPointsLost);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);
  void fullHealthAndMana();

  void addStatus(HeroStatus status, int addedIntensity = 1);
  void removeStatus(HeroStatus status, bool completely);
  bool hasStatus(HeroStatus status) const;
  void setStatusIntensity(HeroStatus status, int newIntensity);
  int getStatusIntensity(HeroStatus status) const;

  void addTrait(HeroTrait trait);
  bool hasTrait(HeroTrait trait) const;
  bool isTraitActive(HeroTrait trait) const;

  std::optional<God> getFollowedDeity() const;
  bool hasBoon(Boon boon) const;

  void removeOneTimeAttackEffects();

  void addDodgeChangePercent(int percent, bool isPermanent);
  int getDodgeChangePercent() const;

  // Methods required to apply side effects of boons and punishments
  void setHitPointsMax(int hitPointsMax);
  void setManaPointsMax(int manaPointsMax);
  void modifyLevelBy(int delta);

  void receive(Item item);
  void receive(Spell spell);
  void convert(Item item);
  void convert(Spell spell);
  void use(Item item);

  void receiveFreeSpell(Spell spell);
  void loseAllItems();

private:
  std::string name;
  HeroStats stats;
  Defence defence;
  Experience experience;
  Inventory inventory;
  Conversion conversion;
  Faith faith;
  std::map<HeroStatus, int> statuses;
  std::vector<HeroTrait> traits;

  void loseHitPoints(int amountPointsLost);
  void propagateStatus(HeroStatus status, int intensity);
  void levelGainedUpdate();
  void apply(PietyChange pietyChange);

  /*
          int gold;
          int conversionPoints;
          */
};
