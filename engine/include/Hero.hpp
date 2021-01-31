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
#include <random>
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
  void gainExperienceForKill(int monsterLevel, bool monsterWasSlowed);
  void gainExperienceForPetrification(bool monsterWasSlowed);
  void gainExperienceNoBonuses(int xpGained);
  void gainLevel();

  bool isDefeated() const;
  int getHitPoints() const;
  int getHitPointsMax() const;
  int getManaPoints() const;
  int getManaPointsMax() const;

  int getBaseDamage() const;
  void changeBaseDamage(int deltaDamagePoints);
  int getDamageBonusPercent() const;
  void changeDamageBonusPercent(int deltaDamageBonusPercent);
  int getDamageVersusStandard() const;
  int getDamageOutputVersus(const Monster& monster) const;

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
  int recoveryMultiplier() const;
  int numSquaresForFullRecovery() const;

  void healHitPoints(int amountPointsHealed, bool mayOverheal = false);
  void loseHitPointsOutsideOfFight(int amountPointsLost);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);
  void refillHealthAndMana();

  void addStatus(HeroStatus status, int addedIntensity = 1);
  void removeStatus(HeroStatus status, bool completely);
  bool hasStatus(HeroStatus status) const;
  int getStatusIntensity(HeroStatus status) const;

  void addStatus(HeroDebuff debuff, int addedIntensity = 1);
  void removeStatus(HeroDebuff debuff, bool completely);
  bool hasStatus(HeroDebuff debuff) const;
  int getStatusIntensity(HeroDebuff debuff) const;

  void addTrait(HeroTrait trait);
  bool hasTrait(HeroTrait trait) const;

  // Adds XP and triggers any faith and item effects
  void monsterKilled(const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning);
  void adjustMomentum(bool increase);
  void removeOneTimeAttackEffects();

  void addDodgeChancePercent(int percent, bool isPermanent);
  int getDodgeChancePercent() const;
  bool predictDodgeNext() const;
  bool tryDodge();

  // Special functions for items. They don't check if the item is actually in the inventory.
  void applyDragonSoul(int manaCosts); // 15% chance to refund mana
  void chargeFireHeart();
  void chargeCrystalBall();

  int gold() const;
  void addGold(int amountAdded);
  bool spendGold(int amountSpent);
  bool buy(Item item);

  Faith& getFaith();
  const Faith& getFaith() const;
  int getPiety() const;

  std::optional<God> getFollowedDeity() const;
  bool hasBoon(Boon boon) const;
  int receivedBoonCount(Boon boon) const;
  int getBoonCosts(Boon boon) const;

  bool followDeity(God god);
  bool request(BoonOrPact boon);
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
  void clearInventory();

  // Inventory management
  std::vector<Inventory::Entry> getItems() const;
  std::vector<Inventory::Entry> getSpells() const;
  const std::vector<Inventory::Entry>& getItemsAndSpells() const;
  bool has(ItemOrSpell itemOrSpell) const;
  bool hasRoomFor(ItemOrSpell itemOrSpell) const;
  bool canAfford(Item item) const;
  void receive(ItemOrSpell itemOrSpell);
  void convert(ItemOrSpell itemOrSpell);
  bool canConvert(ItemOrSpell itemOrSpell) const;
  bool canUse(Item item) const;
  bool canUse(Item item, const Monster& monster) const;
  void use(Item item);
  void use(Item item, Monster& monster);

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
  std::map<HeroDebuff, int> debuffs;
  std::vector<HeroTrait> traits;
  std::optional<PietyChange> collectedPiety;
  std::mt19937 generator;
  bool dodgeNext;
  bool alchemistScrollUsedThisLevel;
  bool namtarsWardUsedThisLevel;
  int momentum;

  void gainExperience(int xpGained, int xpBonuses);
  void drinkHealthPotion();
  void drinkManaPotion();
  int nagaCauldronBonus() const;
  void loseHitPoints(int amountPointsLost);
  void setStatusIntensity(HeroStatus status, int newIntensity);
  void setStatusIntensity(HeroDebuff debuff, int newIntensity);
  void levelGainedUpdate(int newLevel);
  void levelUpRefresh();
  void rerollDodgeNext();
  void applyOrCollect(PietyChange pietyChange);
  void changeStatsFromItem(Item item, bool itemReceived);
};

std::vector<std::string> describe(const Hero& hero);
std::vector<std::string> describe_diff(const Hero& before, const Hero& now);