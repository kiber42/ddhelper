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

  void removeOneTimeAttackEffects();

  void addDodgeChangePercent(int percent, bool isPermanent);
  int getDodgeChangePercent() const;

  int gold() const;
  void addGold(int amountAdded);
  bool spendGold(int amountSpent);

  Faith& getFaith();
  const Faith& getFaith() const;

  std::optional<God> getFollowedDeity() const;
  bool hasBoon(Boon boon) const;
  int receivedBoonCount(Boon boon) const;
  int getBoonCosts(Boon boon) const;

  bool followDeity(God god);
  void desecrate(God altar);

  // Functions to group all piety events belonging to one action.
  // Make sure each call to startPietyCollection is followed by a call to applyCollectedPiety
  void startPietyCollection();
  void collect(PietyChange);
  void applyCollectedPiety();

  // Methods required to apply side effects of worship, boons and punishments
  void setHitPointsMax(int hitPointsMax);
  void setManaPointsMax(int manaPointsMax);
  void changeHitPointsMax(int deltaPoints);
  void changeManaPointsMax(int deltaPoints);
  void changePhysicalResistPercentMax(int deltaPoints);
  void changeMagicalResistPercentMax(int deltaPoints);
  void modifyLevelBy(int delta);
  void addConversionPoints(int points);
  bool lose(Item item);
  void receiveFreeSpell(Spell spell);
  void receiveEnlightenment();
  void loseAllItems();

  // Inventory management
  std::vector<Inventory::Entry> getItems() const;
  std::vector<Inventory::Entry> getSpells() const;
  bool has(Item item) const;
  bool has(Spell spell) const;

  void receive(Item item);
  void receive(Spell spell);
  void convert(Item item);
  void convert(Spell spell);
  bool canUse(Item item) const;
  void use(Item item);

  int getConversionPoints() const;
  int getConversionThreshold() const;

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
  std::optional<PietyChange> collectedPiety;

  void loseHitPoints(int amountPointsLost);
  void propagateStatus(HeroStatus status, int intensity);
  void levelGainedUpdate();
  void applyOrCollect(PietyChange pietyChange);
  void changeStatsFromItem(Item item, bool itemReceived);
};
