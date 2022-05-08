#pragma once

#include "engine/Conversion.hpp"
#include "engine/Defence.hpp"
#include "engine/DungeonSetup.hpp"
#include "engine/Experience.hpp"
#include "engine/Faith.hpp"
#include "engine/HeroClass.hpp"
#include "engine/HeroStats.hpp"
#include "engine/HeroStatus.hpp"
#include "engine/HeroTraits.hpp"
#include "engine/Inventory.hpp"
#include "engine/Items.hpp"
#include "engine/Resources.hpp"

#include <map>
#include <optional>
#include <random>
#include <string>
#include <vector>

class Monster;

class Hero
{
public:
  Hero(HeroClass, HeroRace);
  Hero(HeroClass monsterClass, const std::vector<God>& altarsForGoatperson = {});
  Hero(const DungeonSetup& dungeonSetup, const std::vector<God>& altarsForGoatperson);

  Hero() : Hero(HeroClass::Guard, HeroRace::Human) {}
  Hero(HeroStats, Defence, Experience);

  std::string getName() const;
  void setName(std::string newName);

  unsigned getXP() const;
  unsigned getLevel() const;
  unsigned getPrestige() const;
  unsigned getXPforNextLevel() const;
  unsigned predictExperienceForKill(unsigned monsterLevel, bool monsterWasSlowed) const;
  void gainExperienceForKill(unsigned monsterLevel, bool monsterWasSlowed, Monsters& allMonsters);
  void gainExperienceForPetrification(bool monsterWasSlowed, Monsters& allMonsters);
  void gainExperienceNoBonuses(unsigned xpGained, Monsters& allMonsters);
  void gainLevel(Monsters& allMonsters);

  bool isDefeated() const;
  uint16_t getHitPoints() const;
  uint16_t getHitPointsMax() const;
  uint16_t getManaPoints() const;
  uint16_t getManaPointsMax() const;

  uint16_t getBaseDamage() const;
  void changeBaseDamage(int deltaDamagePoints);
  int getDamageBonusPercent() const;
  void changeDamageBonusPercent(int deltaDamageBonusPercent);
  uint16_t getDamageVersusStandard() const;
  uint16_t getDamageOutputVersus(const Monster& monster) const;

  void addAttackBonus();
  void addHealthBonus();
  void addManaBonus();

  int getPhysicalResistPercent() const;
  int getMagicalResistPercent() const;
  void setPhysicalResistPercent(int physicalResistPercent);
  void setMagicalResistPercent(int magicalResistPercent);
  void changePhysicalResistPercent(int deltaPercent);
  void changeMagicalResistPercent(int deltaPercent);

  bool doesMagicalDamage() const;
  DamageType damageType() const;

  Speed speed() const;
  bool hasInitiativeVersus(const Monster& monster) const;
  bool hasInitiativeVersusIgnoreMonsterSlowed(const Monster& monster) const;
  unsigned predictDamageTaken(unsigned attackerDamageOutput, DamageType damageType) const;
  // Returns false if damage was fully absorbed by damage reduction / resistances
  bool takeDamage(unsigned attackerDamageOutput, DamageType damageType, Monsters& allMonsters);
  void recover(unsigned nSquares, Monsters& allMonsters);
  unsigned recoveryMultiplier() const;
  unsigned numSquaresForFullRecovery() const;

  void healHitPoints(unsigned amountPointsHealed, bool mayOverheal = false);
  void loseHitPointsOutsideOfFight(unsigned amountPointsLost, Monsters& allMonsters);
  void recoverManaPoints(unsigned amountPointsRecovered);
  void loseManaPoints(unsigned amountPointsLost);
  void refillHealthAndMana();
  void addSpiritStrength();

  void add(HeroStatus status, int addedIntensity = 1);
  void reduce(HeroStatus status);
  void reset(HeroStatus status);
  bool has(HeroStatus status) const;
  unsigned getIntensity(HeroStatus status) const;

  void add(HeroDebuff debuff, Monsters& allMonsters, int addedIntensity = 1);
  void reduce(HeroDebuff debuff);
  void reset(HeroDebuff debuff);
  bool has(HeroDebuff debuff) const;
  unsigned getIntensity(HeroDebuff debuff) const;

  void add(HeroTrait trait);
  bool has(HeroTrait trait) const;

  // Adds XP and triggers any faith and item effects
  void monsterKilled(const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning, Monsters& allMonsters, Resources& resources);
  void adjustMomentum(bool increase);
  void removeOneTimeAttackEffects();

  void addDodgeChancePercent(unsigned percent, bool isPermanent);
  unsigned getDodgeChancePercent() const;
  bool predictDodgeNext() const;
  bool tryDodge(Monsters& allMonsters);

  // Trigger effects related to a wall being destroyed
  void wallDestroyed();
  // Trigger effects related to a plant being destroyed
  void plantDestroyed(bool wasPhysicalAttack);
  // Trigger effects related to consuming a blood pool; returns false if hero does not have Sanguine
  bool bloodPoolConsumed();
  // Trigger effects related to casting it on a plant; returns false if the hero cannot cast Imawal.
  // Might trigger Taurog punishment, which gives +10% Magic resistance to enemies.
  bool petrifyPlant(Monsters& allMonsters);

  // Special functions for items. They don't check if the item is actually in the inventory.
  void applyDragonSoul(unsigned manaCosts); // 15% chance to refund mana
  void chargeFireHeart();
  void chargeCrystalBall();

  unsigned gold() const;
  void addGold(unsigned amountReceived);
  bool spendGold(unsigned amountSpent);
  void collectGoldPile();
  int buyingPrice(Item item) const;
  int sellingPrice(Item item) const;
  bool canBuy(Item item) const;
  bool buy(Item item);

  Faith& getFaith();
  const Faith& getFaith() const;
  unsigned getPiety() const;

  std::optional<God> getFollowedDeity() const;
  bool has(Boon boon) const;
  unsigned receivedBoonCount(Boon boon) const;
  int getBoonCosts(Boon boon) const;

  bool followDeity(God god, unsigned numRevealedTiles, Resources& resources);
  bool request(BoonOrPact boon, Monsters& allMonsters, Resources& resources);
  [[nodiscard]] bool desecrate(God altar, Monsters& allMonsters);

  // Functions to group all piety events belonging to one action.
  // Make sure each call to startPietyCollection is followed by a call to applyCollectedPiety or resetCollectedPiety
  // (@see PietyCollection guard object)
  void startPietyCollection();
  void collect(PietyChange);
  void applyCollectedPiety(Monsters& allMonsters);
  void resetCollectedPiety();

  // Methods required to apply side effects of worship, boons and punishments
  void setHitPointsMax(unsigned hitPointsMax);
  void setManaPointsMax(unsigned manaPointsMax);
  void changeHitPointsMax(int deltaPoints);
  void changeManaPointsMax(int deltaPoints);
  void changePhysicalResistPercentMax(int deltaPoints);
  void changeMagicalResistPercentMax(int deltaPoints);
  void modifyLevelBy(int delta);
  void addConversionPoints(unsigned points, Monsters& allMonsters);
  bool lose(Item item);
  [[nodiscard]] bool receiveFreeSpell(Spell spell);
  void receiveEnlightenment(Monsters& allMonsters);
  void clearInventory();

  // Inventory management
  const std::vector<Inventory::Entry>& getItemsAndSpells() const;
  std::vector<std::pair<Item, unsigned>> getItemCounts() const;
  std::vector<std::pair<Spell, unsigned>> getSpellCounts() const;
  std::vector<std::pair<Inventory::Entry, unsigned>> getItemsGrouped() const;
  std::vector<Inventory::Entry> getSpells() const;
  bool has(ItemOrSpell itemOrSpell) const;
  bool hasRoomFor(ItemOrSpell itemOrSpell) const;
  unsigned numFreeSmallInventorySlots() const;
  bool canAfford(Item item) const;
  [[nodiscard]] bool receive(ItemOrSpell itemOrSpell);
  void convert(ItemOrSpell itemOrSpell, Monsters& allMonsters);
  bool canConvert(ItemOrSpell itemOrSpell) const;
  bool canUse(ShopItem item) const;
  bool canUse(BossReward item) const;
  constexpr bool canUse(Item item) const;
  bool canUse(Item item, const Monster& monster) const;
  void use(Potion potion, Monsters& allMonsters);
  void use(ShopItem item, Monsters& allMonsters);
  void use(BossReward item);
  constexpr void use(Item item, Monsters& allMonsters);
  void use(Item item, Monster& monster, Monsters& allMonsters);
  unsigned getFoodCount() const;

  bool useCompressionSealOn(ItemOrSpell itemOrSpell);
  bool useTransmutationSealOn(ItemOrSpell itemOrSpell, Monsters& allMonsters);
  bool useTransmutationSealOnPetrifiedEnemy();
  bool useTransmutationSealOnWallOrPetrifiedPlant();
  bool useTranslocationSealOn(Item shopItem);

  uint8_t getConversionPoints() const;
  uint8_t getConversionThreshold() const;

private:
  std::string name;
  std::vector<HeroTrait> traits;
  HeroStats stats;
  Defence defence;
  Experience experience;
  Inventory inventory;
  Conversion conversion;
  Faith faith;
  std::map<HeroStatus, unsigned> statuses;
  std::map<HeroDebuff, unsigned> debuffs;
  std::optional<PietyChange> collectedPiety;
  std::mt19937 generator{std::random_device{}()};
  bool dodgeNext{false};
  bool alchemistScrollUsedThisLevel{false};
  bool namtarsWardUsedThisLevel{false};

  void gainExperience(unsigned xpGainedTotal, Monsters& allMonsters);
  void drinkHealthPotion();
  void drinkManaPotion();
  // Change health bonus, but not retroactively
  void modifyFutureHealthBonus(int amount);
  unsigned nagaCauldronBonus() const;
  void loseHitPoints(unsigned amountPointsLost, Monsters& allMonsters);
  void setStatusIntensity(HeroStatus status, unsigned newIntensity);
  void levelGainedUpdate(unsigned newLevel, Monsters& allMonsters);
  void levelUpRefresh(Monsters& allMonsters);
  void rerollDodgeNext();
  void applyOrCollect(PietyChange pietyChange, Monsters& allMonsters);
  void applyOrCollectPietyGain(unsigned pointsGained);

  // Add or remove item effects
  void changeStatsImpl(BlacksmithItem item, bool itemReceived);
  void changeStatsImpl(ShopItem item, bool itemReceived);
  void changeStatsImpl(MiscItem item, bool itemReceived);
  void changeStatsImpl(BossReward item, bool itemReceived);
  void changeStatsImpl(TaurogItem item, bool itemReceived);
  void changeStatsFromItem(Item item, bool itemReceived);
};

constexpr bool Hero::canUse(Item item) const
{
  return std::visit(overloaded{
                        [&](const ShopItem& item) { return canUse(item); },
                        [&](const BossReward& item) { return canUse(item); },
                        [](Potion) { return true; },
                        [](const auto&) { return false; },
                    },
                    item);
}

constexpr void Hero::use(Item item, Monsters& allMonsters)
{
  std::visit(overloaded{[&](Potion potion) { use(potion, allMonsters); },
                        [&](const ShopItem& shopItem) { use(shopItem, allMonsters); },
                        [&](BossReward item) { use(item); }, [](const auto&) {}},
             item);
}

[[nodiscard]] std::vector<std::string> describe(const Hero& hero);
[[nodiscard]] std::vector<std::string> describe_diff(const Hero& before, const Hero& now);
