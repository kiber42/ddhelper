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
  explicit Hero(HeroClass = HeroClass::Guard, HeroRace = HeroRace::Human);
  Hero(HeroStats, Defence, Experience);

  std::string getName() const;

  int getXP() const;
  int getLevel() const;
  int getPrestige() const;
  int getXPforNextLevel() const;
  void gainExperienceForKill(int monsterLevel, bool monsterWasSlowed, Monsters& allMonsters);
  void gainExperienceForPetrification(bool monsterWasSlowed, Monsters& allMonsters);
  void gainExperienceNoBonuses(int xpGained, Monsters& allMonsters);
  void gainLevel(Monsters& allMonsters);

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
  void takeDamage(int attackerDamageOutput, bool isMagicalDamage, Monsters& allMonsters);
  void recover(int nSquares);
  int recoveryMultiplier() const;
  int numSquaresForFullRecovery() const;

  void healHitPoints(int amountPointsHealed, bool mayOverheal = false);
  void loseHitPointsOutsideOfFight(int amountPointsLost, Monsters& allMonsters);
  void recoverManaPoints(int amountPointsRecovered);
  void loseManaPoints(int amountPointsLost);
  void refillHealthAndMana();
  void addSpiritStrength();

  void addStatus(HeroStatus status, int addedIntensity = 1);
  void reduceStatus(HeroStatus status);
  void resetStatus(HeroStatus status);
  bool hasStatus(HeroStatus status) const;
  int getStatusIntensity(HeroStatus status) const;

  void addStatus(HeroDebuff debuff, Monsters& allMonsters, int addedIntensity = 1);
  void reduceStatus(HeroDebuff debuff);
  void resetStatus(HeroDebuff debuff);
  bool hasStatus(HeroDebuff debuff) const;
  int getStatusIntensity(HeroDebuff debuff) const;

  void addTrait(HeroTrait trait);
  bool hasTrait(HeroTrait trait) const;

  // Adds XP and triggers any faith and item effects
  void monsterKilled(const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning, Monsters& allMonsters);
  void adjustMomentum(bool increase);
  void removeOneTimeAttackEffects();

  void addDodgeChancePercent(int percent, bool isPermanent);
  int getDodgeChancePercent() const;
  bool predictDodgeNext() const;
  bool tryDodge(Monsters& allMonsters);

  // Special functions for items. They don't check if the item is actually in the inventory.
  void applyDragonSoul(int manaCosts); // 15% chance to refund mana
  void chargeFireHeart();
  void chargeCrystalBall();

  int gold() const;
  void addGold(int amountAdded);
  bool spendGold(int amountSpent);
  int buyingPrice(Item item) const;
  int sellingPrice(Item item) const;
  bool buy(Item item);

  Faith& getFaith();
  const Faith& getFaith() const;
  int getPiety() const;

  std::optional<God> getFollowedDeity() const;
  bool hasBoon(Boon boon) const;
  int receivedBoonCount(Boon boon) const;
  int getBoonCosts(Boon boon) const;

  bool followDeity(God god);
  bool request(BoonOrPact boon, Monsters& allMonsters);
  void desecrate(God altar, Monsters& allMonsters);

  // Functions to group all piety events belonging to one action.
  // Make sure each call to startPietyCollection is followed by a call to applyCollectedPiety
  void startPietyCollection();
  void collect(PietyChange);
  void applyCollectedPiety(Monsters& allMonsters);

  // Methods required to apply side effects of worship, boons and punishments
  void setHitPointsMax(int hitPointsMax);
  void setManaPointsMax(int manaPointsMax);
  void changeHitPointsMax(int deltaPoints);
  void changeManaPointsMax(int deltaPoints);
  void changePhysicalResistPercentMax(int deltaPoints);
  void changeMagicalResistPercentMax(int deltaPoints);
  void modifyLevelBy(int delta);
  void addConversionPoints(int points, Monsters& allMonsters);
  bool lose(Item item);
  void receiveFreeSpell(Spell spell);
  void receiveEnlightenment(Monsters& allMonsters);
  void clearInventory();

  // Inventory management
  const std::vector<Inventory::Entry>& getItemsAndSpells() const;
  std::vector<std::pair<Item, int>> getItemCounts() const;
  std::vector<std::pair<Spell, int>> getSpellCounts() const;
  std::vector<std::pair<Inventory::Entry, int>> getItemsGrouped() const;
  std::vector<Inventory::Entry> getSpells() const;
  bool has(ItemOrSpell itemOrSpell) const;
  bool hasRoomFor(ItemOrSpell itemOrSpell) const;
  bool canAfford(Item item) const;
  void receive(ItemOrSpell itemOrSpell);
  void convert(ItemOrSpell itemOrSpell, Monsters& allMonsters);
  bool canConvert(ItemOrSpell itemOrSpell) const;
  bool canUse(Item item) const;
  bool canUse(Item item, const Monster& monster) const;
  void use(Item item, Monsters& allMonsters);
  void use(Item item, Monster& monster, Monsters& allMonsters);

  bool useCompressionSealOn(ItemOrSpell itemOrSpell);
  bool useTransmutationSealOn(ItemOrSpell itemOrSpell, Monsters& allMonsters);
  bool useTransmutationSealOnPetrifiedEnemy();
  bool useTransmutationSealOnWallOrPetrifiedPlant();
  bool useTranslocationSealOn(Item shopItem);

  int getConversionPoints() const;
  int getConversionThreshold() const;

private:
  std::string name;
  std::vector<HeroTrait> traits;
  HeroStats stats;
  Defence defence;
  Experience experience;
  Inventory inventory;
  Conversion conversion;
  Faith faith;
  std::map<HeroStatus, int> statuses;
  std::map<HeroDebuff, int> debuffs;
  std::optional<PietyChange> collectedPiety;
  std::mt19937 generator;
  bool dodgeNext;
  bool alchemistScrollUsedThisLevel;
  bool namtarsWardUsedThisLevel;

  void gainExperience(int xpGained, int xpBonuses, Monsters& allMonsters);
  void drinkHealthPotion();
  void drinkManaPotion();
  int nagaCauldronBonus() const;
  void loseHitPoints(int amountPointsLost, Monsters& allMonsters);
  void setStatusIntensity(HeroStatus status, int newIntensity);
  void levelGainedUpdate(int newLevel, Monsters& allMonsters);
  void levelUpRefresh(Monsters& allMonsters);
  void rerollDodgeNext();
  void applyOrCollect(PietyChange pietyChange, Monsters& allMonsters);
  void changeStatsFromItem(Item item, bool itemReceived);
};

std::vector<std::string> describe(const Hero& hero);
std::vector<std::string> describe_diff(const Hero& before, const Hero& now);
