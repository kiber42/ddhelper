#include "engine/Hero.hpp"

#include "engine/Clamp.hpp"
#include "engine/Experience.hpp"
#include "engine/Items.hpp"
#include "engine/Magic.hpp"
#include "engine/Monster.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

Hero::Hero(HeroClass heroClass, HeroRace heroRace)
  : Hero(DungeonSetup{heroClass, heroRace}, {})
{
}

Hero::Hero(HeroClass monsterClass, const std::vector<God>& altarsForGoatperson)
  : Hero(DungeonSetup{monsterClass}, altarsForGoatperson)
{
}

Hero::Hero(const DungeonSetup& setup, const std::vector<God>& altarsForGoatperson)
  : name(isMonsterClass(setup.heroClass) ? toString(setup.heroClass)
                                         : (toString(setup.heroRace) + std::string(" ") + toString(setup.heroClass)))
  , traits(startingTraits(setup.heroClass))
  , defence(0_physicalresist, 0_magicalresist, 65_physicalresist, 65_magicalresist)
  , experience()
  , inventory(setup)
  , conversion(setup)
  , faith(setup.altar)
  , generator(std::random_device{}())
{
  if (has(HeroTrait::Veteran))
    experience = Experience(Experience::IsVeteran{});
  if (has(HeroTrait::Dangerous))
    stats = HeroStats(HeroStats::IsDangerous{});
  if (has(HeroTrait::RegalSize))
    stats = HeroStats(HeroStats::RegalSize{});

  if (has(HeroTrait::PitDog))
    add(HeroStatus::DeathProtection);
  if (has(HeroTrait::Mageslay))
    changeDamageBonusPercent(+20);
  if (has(HeroTrait::Spellkill))
    defence.set(50_magicalresist);
  if (has(HeroTrait::ArcaneKnowledge))
    stats.setManaPointsMax(stats.getManaPointsMax() + 5_MP);
  if (has(HeroTrait::Insane))
  {
    setStatusIntensity(HeroStatus::Sanguine, 20);
    stats.setManaPointsMax(stats.getManaPointsMax() - 3_MP);
  }
  if (has(HeroTrait::Dexterous))
    add(HeroStatus::FirstStrikePermanent);
  if (has(HeroTrait::Evasive))
    addDodgeChancePercent(20, true);
  if (has(HeroTrait::PoisonedBlade))
    changeDamageBonusPercent(-20);
  if (has(HeroTrait::GoodHealth))
  {
    stats.addHealthBonus(Level{1});
    stats.addHealthBonus(Level{1});
    stats.addHealthBonus(Level{1});
    stats.healHitPoints(3_HP, false);
  }
  if (has(HeroTrait::HandToHand))
  {
    stats.setBaseDamage(3_damage);
    changeDamageBonusPercent(-30);
  }
  if (has(HeroTrait::DiamondBody))
  {
    defence.set(50_physicalresist);
    defence.setMax(75_physicalresist);
  }
  if (has(HeroTrait::HolyShield))
    defence.set(25_physicalresist);
  if (has(HeroTrait::Scars))
  {
    add(HeroStatus::PoisonImmune);
    add(HeroStatus::ManaBurnImmune);
    add(HeroStatus::CurseImmune);
  }
  if (has(HeroTrait::Undead))
  {
    add(HeroStatus::PoisonImmune);
    add(HeroStatus::ManaBurnImmune);
  }
  if (has(HeroTrait::EternalThirst))
  {
    add(HeroStatus::Sanguine, 5);
    add(HeroStatus::LifeSteal, 1);
  }
  if (has(HeroTrait::DragonBreath))
    add(HeroStatus::MagicalAttack);
  if (has(HeroTrait::DragonTail))
    add(HeroStatus::Knockback, 20);
  if (has(HeroTrait::DragonStature))
    stats.setHitPointsMax(stats.getHitPointsMax() * 2);
  if (has(HeroTrait::AzureBody))
  {
    defence.set(25_physicalresist);
    add(HeroStatus::DeathGazeImmune);
    changeDamageBonusPercent(-50);
  }
  if (has(HeroTrait::SapphireLocks))
    add(HeroStatus::Poisonous, 5);
  if (has(HeroTrait::AmethystStare))
    add(HeroStatus::DeathGaze, 10);
  if (has(HeroTrait::RegalHygiene))
    add(HeroStatus::CorrosiveStrike);

  if (has(HeroTrait::Scapegoat))
  {
    assert(!altarsForGoatperson.empty());
    faith.makeGoatperson(altarsForGoatperson);
  }

  // TODO: Move gold pile size to MapResources? (+ remove this trait)
  if (setup.modifiers.count(ThievesModifier::BlackMarket))
    add(HeroTrait::BlackMarket);

  for (auto& item : setup.startingEquipment)
    changeStatsFromItem(item, true);
}

Hero::Hero(HeroStats stats, Defence defence, Experience experience)
  : name("Hero")
  , traits()
  , stats(std::move(stats))
  , defence(std::move(defence))
  , experience(std::move(experience))
  , inventory(DungeonSetup{})
  , conversion(DungeonSetup{})
  , faith()
  , statuses()
  , collectedPiety()
  , generator(std::random_device{}())
{
}

std::string Hero::getName() const
{
  return name;
}

void Hero::setName(std::string newName)
{
  name = newName;
}

unsigned Hero::getXP() const
{
  return experience.getXP().get();
}

unsigned Hero::getLevel() const
{
  return experience.getLevel().get();
}

unsigned Hero::getPrestige() const
{
  return experience.getPrestige();
}

unsigned Hero::getXPforNextLevel() const
{
  return experience.getXPforNextLevel().get();
}

unsigned Hero::predictExperienceForKill(unsigned monsterLevel, bool monsterWasSlowed) const
{
  auto xp = Experience::forHeroAndMonsterLevels(experience.getLevel(), Level{monsterLevel});
  if (has(HeroStatus::ExperienceBoost))
    xp += xp / 2;
  xp += ExperiencePoints{getIntensity(HeroStatus::Learning)};
  if (monsterWasSlowed)
    ++xp;
  if (has(HeroTrait::Veteran))
    ++xp;
  if (has(ShopItem::BalancedDagger) && getLevel() == monsterLevel)
    xp += 2_xp;
  return xp.get();
}

void Hero::gainExperienceForKill(unsigned monsterLevel, bool monsterWasSlowed, Monsters& allMonsters)
{
  gainExperience(predictExperienceForKill(monsterLevel, monsterWasSlowed), allMonsters);
  reset(HeroStatus::ExperienceBoost);
}

void Hero::gainExperienceForPetrification(bool monsterWasSlowed, Monsters& allMonsters)
{
  if (monsterWasSlowed)
    gainExperience(1, allMonsters);
}

void Hero::gainExperienceNoBonuses(unsigned xpGained, Monsters& allMonsters)
{
  gainExperience(xpGained, allMonsters);
}

void Hero::gainExperience(unsigned xpGainedTotal, Monsters& allMonsters)
{
  auto level = getLevel();
  const auto prestige = getPrestige();
  experience.gain(ExperiencePoints{xpGainedTotal});
  const bool levelUp = getLevel() > level || getPrestige() > prestige;
  while (getLevel() > level)
  {
    ++level;
    levelGainedUpdate(level, allMonsters);
  }
  if (levelUp)
    levelUpRefresh(allMonsters);
}

void Hero::gainLevel(Monsters& allMonsters)
{
  const unsigned initialLevel = getLevel();
  experience.gainLevel();
  if (getLevel() > initialLevel)
    levelGainedUpdate(getLevel(), allMonsters);
  levelUpRefresh(allMonsters);
}

bool Hero::isDefeated() const
{
  return stats.isDefeated();
}

uint16_t Hero::getHitPoints() const
{
  return stats.getHitPoints().get();
}

uint16_t Hero::getHitPointsMax() const
{
  return stats.getHitPointsMax().get();
}

uint16_t Hero::getManaPoints() const
{
  return stats.getManaPoints().get();
}

uint16_t Hero::getManaPointsMax() const
{
  return stats.getManaPointsMax().get();
}

void Hero::drinkHealthPotion()
{
  const bool hasNagaCauldron = has(BossReward::NagaCauldron);
  uint16_t percentHealed = has(HeroTrait::GoodDrink) ? 100 : 40;
  if (hasNagaCauldron)
    percentHealed += nagaCauldronBonus();
  stats.healHitPoints(stats.getHitPointsMax().percentage(percentHealed), hasNagaCauldron);
  reset(HeroDebuff::Poisoned);
  if (has(HeroTrait::Survivor))
    stats.recoverManaPoints(stats.getManaPointsMax().percentage(20));
  if (has(HeroTrait::Colourants))
  {
    add(HeroStatus::Healthform);
    reset(HeroStatus::Manaform);
  }
}

void Hero::drinkManaPotion()
{
  const bool hasNagaCauldron = has(BossReward::NagaCauldron);
  uint16_t percentRestored = 40;
  if (has(HeroTrait::PowerHungry))
  {
    percentRestored = 60;
    add(HeroStatus::Sanguine, 2);
  }
  if (hasNagaCauldron)
    percentRestored += nagaCauldronBonus();
  stats.recoverManaPoints(stats.getManaPointsMax().percentage(percentRestored));
  reset(HeroDebuff::ManaBurned);
  if (has(HeroTrait::Courageous))
    add(HeroStatus::Might);
  if (has(HeroTrait::Survivor))
    stats.healHitPoints(stats.getHitPointsMax().percentage(20), false);
  if (has(HeroTrait::Colourants))
  {
    add(HeroStatus::Manaform);
    reset(HeroStatus::Healthform);
  }
}

unsigned Hero::nagaCauldronBonus() const
{
  return 5u * (static_cast<unsigned>(has(HeroDebuff::Poisoned)) + static_cast<unsigned>(has(HeroDebuff::ManaBurned)) +
               static_cast<unsigned>(has(HeroDebuff::Corroded)) + static_cast<unsigned>(has(HeroDebuff::Weakened)) +
               static_cast<unsigned>(has(HeroDebuff::Cursed)));
}

uint16_t Hero::getBaseDamage() const
{
  auto damage = stats.getBaseDamage() + DamagePoints{getIntensity(HeroStatus::SpiritStrength)};
  if (has(HeroTrait::Additives))
    damage += DamagePoints{getIntensity(HeroStatus::Might)};
  const auto weakened = DamagePoints{getIntensity(HeroDebuff::Weakened)};
  return damage > weakened ? (damage - weakened).get() : 1u;
}

namespace
{
  template <class NumericType>
  constexpr NumericType changeHelper(NumericType current, int delta, NumericType min)
  {
    assert(current >= min);
    if (delta > 0)
      return current + NumericType{delta};
    if (const auto decrease = NumericType{-delta}; decrease < current)
      return current - decrease;
    return min;
  }
} // namespace

void Hero::changeBaseDamage(int deltaDamagePoints)
{
  const auto newDamage = changeHelper(stats.getBaseDamage(), deltaDamagePoints, 1_damage);
  stats.setBaseDamage(newDamage);
}

int Hero::getDamageBonusPercent() const
{
  auto bonus = stats.getDamageBonus().in_percent();
  bonus += 30 * getIntensity(HeroStatus::Might);
  if (has(HeroTrait::Determined) && stats.getHitPoints() * 2 < stats.getHitPointsMax())
    bonus += 30;
  return bonus;
}

void Hero::changeDamageBonusPercent(int deltaDamageBonusPercent)
{
  stats.setDamageBonus(DamageBonus{stats.getDamageBonus().in_percent() + deltaDamageBonusPercent});
}

uint16_t Hero::getDamageVersusStandard() const
{
  const int multiplier = 100 + getDamageBonusPercent();
  if (multiplier > 0)
    return getBaseDamage() * static_cast<uint16_t>(multiplier) / 100u;
  else
    return 0;
}

uint16_t Hero::getDamageOutputVersus(const Monster& monster) const
{
  const auto standardDamage = getDamageVersusStandard();
  auto damage = standardDamage;
  if (has(HeroTrait::Bloodlust) && monster.getLevel() > getLevel())
    damage += standardDamage * 2 / 10;
  if (has(HeroTrait::Stabber) && monster.getHitPoints() >= monster.getHitPointsMax())
    damage += standardDamage * 3 / 10;
  if (has(HeroTrait::PoisonedBlade) && monster.isPoisoned())
    damage += standardDamage * 4 / 10;
  if (has(HeroTrait::GoodGolly) && monster.has(MonsterTrait::Undead))
    damage += getBaseDamage();
  if (has(HeroTrait::SwiftHand) && getLevel() > monster.getLevel())
    damage = clampedTo<uint16_t>(10 * monster.getHitPoints());
  return damage;
}

void Hero::addAttackBonus()
{
  changeDamageBonusPercent(+10);
}

void Hero::addHealthBonus()
{
  stats.addHealthBonus(Level{getLevel()});
}

void Hero::addManaBonus()
{
  stats.setManaPointsMax(stats.getManaPointsMax() + 1_MP);
}

void Hero::modifyFutureHealthBonus(int amount)
{
  for (int i = 0; i < amount; ++i)
    stats.addFutureHealthBonus();
  for (int i = 0; i < -amount; ++i)
    stats.reduceHealthBonus();
}

int Hero::getPhysicalResistPercent() const
{
  return defence.getPhysicalResist().in_percent();
}

int Hero::getMagicalResistPercent() const
{
  return defence.getMagicalResist().in_percent();
}

void Hero::setPhysicalResistPercent(int physicalResistPercent)
{
  defence.set(PhysicalResist{physicalResistPercent});
}

void Hero::setMagicalResistPercent(int magicalResistPercent)
{
  defence.set(MagicalResist{magicalResistPercent});
}

void Hero::changePhysicalResistPercent(int deltaPercent)
{
  defence.changePhysicalResistPercent(deltaPercent);
}

void Hero::changeMagicalResistPercent(int deltaPercent)
{
  defence.changeMagicalResistPercent(deltaPercent);
}

bool Hero::doesMagicalDamage() const
{
  return has(HeroStatus::ConsecratedStrike) || has(HeroStatus::MagicalAttack);
}

DamageType Hero::damageType() const
{
  return doesMagicalDamage() ? DamageType::Magical
                             : has(HeroStatus::PiercePhysical) ? DamageType::Piercing : DamageType::Physical;
}

bool Hero::hasInitiativeVersus(const Monster& monster) const
{
  if (has(HeroStatus::Reflexes) || (has(HeroTrait::SwiftHand) && getLevel() > monster.getLevel()))
    return true;

  const bool firstStrike = has(HeroStatus::FirstStrikePermanent) || has(HeroStatus::FirstStrikeTemporary);
  const bool heroFast = firstStrike && !has(HeroStatus::SlowStrike);
  const bool monsterFast = monster.has(MonsterTrait::FirstStrike) && !monster.isSlowed();
  if (heroFast || monsterFast)
    return !monsterFast;

  const bool heroSlow = !firstStrike && has(HeroStatus::SlowStrike);
  const bool monsterSlow = monster.isSlowed();
  if (heroSlow || monsterSlow)
    return !heroSlow;

  return getLevel() > monster.getLevel();
}

unsigned Hero::predictDamageTaken(unsigned attackerDamageOutput, DamageType damageType) const
{
  const auto reduction = getIntensity(HeroStatus::DamageReduction);
  if (reduction < attackerDamageOutput)
    return defence.predictDamageTaken(DamagePoints{attackerDamageOutput - reduction}, damageType, 0_burn).get();
  else
    return 0;
}

void Hero::loseHitPoints(unsigned amountPointsLost, Monsters& allMonsters)
{
  stats.loseHitPointsWithoutDeathProtection(HitPoints{amountPointsLost});
  if (stats.getHitPoints() == 0_HP && has(HeroStatus::DeathProtection))
  {
    stats.barelySurvive();
    reset(HeroStatus::DeathProtection);
    applyOrCollect(faith.deathProtectionTriggered(), allMonsters);
  }
}

bool Hero::takeDamage(unsigned attackerDamageOutput, DamageType damageType, Monsters& allMonsters)
{
  const auto damagePoints = predictDamageTaken(attackerDamageOutput, damageType);
  loseHitPoints(damagePoints, allMonsters);
  if (damagePoints > 0 && has(HeroStatus::Schadenfreude))
  {
    recoverManaPoints(damagePoints);
    reset(HeroStatus::Schadenfreude);
  }
  reset(HeroStatus::StoneSkin);
  return damagePoints > 0;
}

void Hero::recover(unsigned nSquares, Monsters& allMonsters)
{
  const bool manaform = has(HeroStatus::Manaform);
  const bool healthform = has(HeroStatus::Healthform);
  const bool recoverHealth = !manaform && !has(HeroDebuff::Poisoned);
  const bool recoverMana = !healthform && !has(HeroStatus::Exhausted) && !has(HeroDebuff::ManaBurned);
  const auto nFoodMissing = has(HeroTrait::Herbivore) ? inventory.tryConsumeFood(nSquares) : 0u;
  const auto nRecover = nSquares - nFoodMissing;
  if (recoverHealth)
    stats.healHitPoints(HitPoints{nRecover * recoveryMultiplier()}, false);
  if (recoverMana)
    stats.recoverManaPoints(ManaPoints{nRecover * (manaform ? 2u : 1u)});
  // Lose health when uncovering without food; this may kill the hero. However, death protection will trigger at most
  // once even if multiple tiles are uncovered after running out of food and health.
  if (nFoodMissing > 0)
    loseHitPointsOutsideOfFight(getLevel() * nFoodMissing, allMonsters);
}

unsigned Hero::recoveryMultiplier() const
{
  auto multiplier = getLevel();
  if (has(HeroStatus::Healthform))
    multiplier *= 2;
  if (has(HeroTrait::Discipline))
    multiplier *= 2;
  if (has(ShopItem::BloodySigil))
    multiplier += 1;
  if (has(HeroTrait::Damned))
    multiplier = 1;
  return multiplier;
}

unsigned Hero::numSquaresForFullRecovery() const
{
  auto numHP = 0u;
  if (!has(HeroDebuff::Poisoned))
  {
    const auto multiplier = recoveryMultiplier();
    numHP = (getHitPointsMax() - getHitPoints() + (multiplier - 1) /* always round up */) / multiplier;
  }
  const auto numMP = has(HeroDebuff::ManaBurned) ? 0u : getManaPointsMax() - getManaPoints();
  const auto numSquares = has(HeroTrait::Damned) ? numHP + numMP : std::max(numHP, numMP);
  if (has(HeroTrait::Herbivore))
  {
    const auto foodCount = inventory.getFoodCount();
    if (foodCount < numSquares)
      return foodCount;
  }
  return numSquares;
}

void Hero::healHitPoints(unsigned amountPointsHealed, bool mayOverheal)
{
  stats.healHitPoints(HitPoints{amountPointsHealed}, mayOverheal);
}

void Hero::loseHitPointsOutsideOfFight(unsigned amountPointsLost, Monsters& allMonsters)
{
  loseHitPoints(amountPointsLost, allMonsters);
}

void Hero::recoverManaPoints(unsigned amountPointsRecovered)
{
  stats.recoverManaPoints(ManaPoints{amountPointsRecovered});
}

void Hero::loseManaPoints(unsigned amountPointsLost)
{
  stats.loseManaPoints(ManaPoints{amountPointsLost});
}

void Hero::refillHealthAndMana()
{
  stats.refresh();
}

void Hero::addSpiritStrength()
{
  const auto mp = getManaPoints();
  const auto newSpiritStrength = getLevel() + mp;
  auto iter = statuses.find(HeroStatus::SpiritStrength);
  if (iter == end(statuses))
    statuses[HeroStatus::SpiritStrength] = newSpiritStrength;
  else
    iter->second = std::max(newSpiritStrength, iter->second);
  loseManaPoints(mp);
}

void Hero::add(HeroStatus status, int addedIntensity)
{
  const int newIntensity = static_cast<int>(statuses[status]) + addedIntensity;
  assert(newIntensity >= 0);
  setStatusIntensity(status, static_cast<unsigned>(newIntensity));
}

void Hero::reduce(HeroStatus status)
{
  if (has(status))
    setStatusIntensity(status, statuses[status] - 1);
}

void Hero::reset(HeroStatus status)
{
  setStatusIntensity(status, 0);
}

bool Hero::has(HeroStatus status) const
{
  return getIntensity(status) > 0;
}

void Hero::setStatusIntensity(HeroStatus status, unsigned newIntensity)
{
  assert(status != HeroStatus::Exhausted && "Exhausted status is computed on the fly");
  assert(status != HeroStatus::Might || newIntensity <= 1 || has(HeroTrait::Additives));
  if (newIntensity > 1 && !canHaveMultiple(status))
    newIntensity = 1;

  const auto oldIntensity = std::exchange(statuses[status], newIntensity);
  if (newIntensity == oldIntensity)
    return;

  if (status == HeroStatus::Momentum)
  {
    const auto delta = static_cast<int>(newIntensity) - static_cast<int>(oldIntensity);
    changeDamageBonusPercent(delta);
  }

  if (newIntensity > 0)
  {
    if (status == HeroStatus::CurseImmune)
      reset(HeroDebuff::Cursed);
    else if (status == HeroStatus::ManaBurnImmune)
      reset(HeroDebuff::ManaBurned);
    else if (status == HeroStatus::PoisonImmune)
      reset(HeroDebuff::Poisoned);
  }
  else if (newIntensity == 0)
    statuses.erase(status);

  if (status == HeroStatus::DodgePermanent || status == HeroStatus::DodgeTemporary)
    rerollDodgeNext();

  if (status == HeroStatus::StoneSkin)
    defence.set(StoneSkinLayers{newIntensity});
}

unsigned Hero::getIntensity(HeroStatus status) const
{
  // Compute Exhausted status when needed instead of updating it all the time
  if (status == HeroStatus::Exhausted)
    return has(HeroTrait::Damned) && getHitPoints() < getHitPointsMax();

  auto iter = statuses.find(status);
  return iter != statuses.end() ? iter->second : 0;
}

void Hero::add(HeroDebuff debuff, Monsters& allMonsters, int addedIntensity)
{
  if (addedIntensity <= 0)
  {
    if (addedIntensity < 0)
    {
      // For convenience, allow to call add with -1 instead of reduce
      assert(addedIntensity == -1);
      reduce(debuff);
    }
    return;
  }

  // Reject changes if there is a corresponding immunity
  if ((debuff == HeroDebuff::Cursed && has(HeroStatus::CurseImmune)) ||
      (debuff == HeroDebuff::ManaBurned && has(HeroStatus::ManaBurnImmune)) ||
      (debuff == HeroDebuff::Poisoned && has(HeroStatus::PoisonImmune)))
    return;

  auto& intensity = debuffs[debuff];

  // Mana burn is special: Although one cannot have multiple layers, adding it will always set mana to 0
  if (debuff == HeroDebuff::ManaBurned)
  {
    PietyChange pietyChange;
    if (intensity == 0)
      pietyChange += faith.becameManaBurned();
    const auto mp = getManaPoints();
    pietyChange += faith.manaPointsBurned(mp);
    loseManaPoints(mp);
    applyOrCollect(pietyChange, allMonsters);
  }

  if (intensity != 0 && !canHaveMultiple(debuff))
    return;

  intensity += static_cast<unsigned>(addedIntensity);

  if (debuff == HeroDebuff::Poisoned)
    applyOrCollect(faith.becamePoisoned(), allMonsters);
  else if (debuff == HeroDebuff::Corroded)
    defence.set(CorrosionAmount{intensity});
  else if (debuff == HeroDebuff::Cursed)
    defence.setCursed(true);
}

void Hero::reduce(HeroDebuff debuff)
{
  auto iter = debuffs.find(debuff);
  if (iter == debuffs.end() || iter->second == 0)
    return;

  const auto newIntensity = iter->second - 1;

  if (newIntensity == 0)
  {
    reset(debuff);
    return;
  }

  debuffs[debuff] = newIntensity;

  if (debuff == HeroDebuff::Corroded)
    defence.set(CorrosionAmount{newIntensity});
}

void Hero::reset(HeroDebuff debuff)
{
  if (debuffs.erase(debuff))
  {
    if (debuff == HeroDebuff::Corroded)
      defence.set(0_corrosion);
    else if (debuff == HeroDebuff::Cursed)
      defence.setCursed(false);
  }
}

bool Hero::has(HeroDebuff debuff) const
{
  return getIntensity(debuff) > 0;
}

unsigned Hero::getIntensity(HeroDebuff debuff) const
{
  auto iter = debuffs.find(debuff);
  return iter != debuffs.end() ? iter->second : 0;
}

void Hero::add(HeroTrait trait)
{
  if (has(trait))
  {
    assert(false);
    return;
  }
  traits.emplace_back(trait);
}

bool Hero::has(HeroTrait trait) const
{
  return std::find(begin(traits), end(traits), trait) != end(traits);
}

void Hero::monsterKilled(
    const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning, Monsters& allMonsters, Resources& resources)
{
  assert(monster.isDefeated());
  add(HeroDebuff::Cursed, allMonsters, monster.has(MonsterTrait::CurseBearer) ? 1 : -1);
  applyOrCollect(faith.monsterKilled(monster, getLevel(), monsterWasBurning), allMonsters);
  if (monster.grantsXP())
  {
    const auto monsterLevel = monster.getLevel();
    gainExperienceForKill(monsterLevel, monsterWasSlowed, allMonsters);
    if (has(ShopItem::GlovesOfMidas))
      ++inventory.gold;
    if (monsterLevel == 10)
      inventory.gold += 25;
    if (has(ShopItem::BlueBead) && !has(HeroDebuff::ManaBurned))
      recoverManaPoints(1);
    if (has(HeroTrait::Herbivore))
    {
      if (!inventory.addFood(9))
        resources().onGround.push_back(MiscItem::FoodStack);
    }
    if (has(ShopItem::StoneSigil))
      faith.gainPiety(1);
  }
  if (!monster.has(MonsterTrait::Bloodless))
    ++resources().numBloodPools;
  if (has(HeroTrait::SapphireLocks))
    ++resources().numWalls;
  if (resources.uses(Ruleset::MonsterMachine1))
  {
    auto boss = std::find_if(rbegin(allMonsters), rend(allMonsters),
                             [](const auto& monster) { return monster.getLevel() == 10; });
    if (boss != rend(allMonsters))
      boss->corrode();
    const auto reward = [rewardIndex = std::uniform_int_distribution<>(0, 3)(generator),
                         corrodeReward = !monster.has(MonsterTrait::Bloodless)]() -> Item {
      switch (rewardIndex)
      {
      default:
        assert(false);
      case 0:
        return corrodeReward ? MiscItem::WispGemCorroded : MiscItem::WispGem;
      case 1:
        return corrodeReward ? MiscItem::WallCruncherCorroded : MiscItem::WallCruncher;
      case 2:
        return corrodeReward ? MiscItem::CharmCorroded : MiscItem::Charm;
      case 3:
        return corrodeReward ? Potion::CourageJuiceCorroded : Potion::CourageJuice;
      }
    }();
    resources().onGround.emplace_back(reward);
  }
}

void Hero::adjustMomentum(bool increase)
{
  if (has(HeroTrait::Momentum))
  {
    if (increase)
      add(HeroStatus::Momentum, 15);
    else
    {
      const auto momentum = static_cast<int>(getIntensity(HeroStatus::Momentum));
      const auto delta = -(momentum + 1) / 2;
      add(HeroStatus::Momentum, delta);
    }
  }
}

void Hero::removeOneTimeAttackEffects()
{
  reset(HeroStatus::ConsecratedStrike);
  reset(HeroStatus::CrushingBlow);
  reset(HeroStatus::Might);
  reset(HeroStatus::ByssepsStacks);
  reset(HeroStatus::SpiritStrength);
  reset(HeroStatus::FirstStrikeTemporary);
  reset(HeroStatus::Reflexes);

  if (inventory.triswordUsed())
    changeBaseDamage(-1);
}

void Hero::levelGainedUpdate(unsigned newLevel, Monsters& allMonsters)
{
  assert(stats.getHealthBonus() >= -10);
  const auto newHpMax = stats.getHitPointsMax() + HitPoints{10 + stats.getHealthBonus()};
  stats.setHitPointsMax(newHpMax);
  const int addedBaseDamage = [&] {
    if (has(HeroTrait::HandToHand))
      return 3;
    else if (has(HeroTrait::RegalSize))
      return 1;
    return 5;
  }();
  changeBaseDamage(addedBaseDamage);
  if (has(HeroTrait::RegalHygiene))
    add(HeroStatus::CorrosiveStrike);
  if (has(ShopItem::Platemail))
    add(HeroStatus::DamageReduction, 2);
  if (has(ShopItem::MartyrWraps))
  {
    add(HeroDebuff::Corroded, allMonsters);
    // TODO: Only applies to visible monsters
    for (auto& monster : allMonsters)
      monster.corrode();
  }
  if (has(ShopItem::MagePlate) && newLevel % 2 == 1)
  {
    changeManaPointsMax(1);
    changeDamageBonusPercent(-5);
  }
  alchemistScrollUsedThisLevel = false;
  namtarsWardUsedThisLevel = false;
  applyOrCollect(faith.levelGained(), allMonsters);
  adjustMomentum(false);
}

void Hero::levelUpRefresh(Monsters& allMonsters)
{
  if (has(MiscItem::PatchesTheTeddy) && !has(HeroStatus::Pessimist))
  {
    // Random positive effect
    switch (std::uniform_int_distribution<>(0, 4)(generator))
    {
    case 0:
      setHitPointsMax(getHitPointsMax() + 3);
      break;
    case 1:
      addGold(3);
      break;
    case 2:
      changeDamageBonusPercent(5);
      break;
    case 3:
      setManaPointsMax(getManaPoints() + 1);
      break;
    case 4:
      changePhysicalResistPercent(4);
      changeMagicalResistPercent(4);
      break;
    }
    // Random negative effect
    switch (std::uniform_int_distribution<>(0, 2)(generator))
    {
    case 0:
      add(HeroDebuff::Poisoned, allMonsters);
      break;
    case 1:
      add(HeroDebuff::ManaBurned, allMonsters);
      break;
    case 2:
      // not implemented: random teleport
      break;
    case 3:
      // not implemented: grid reveal (rare?)
      break;
    }
  }

  if (has(HeroTrait::Scapegoat))
    faith.selectNextDeityForGoatperson();

  if (!has(HeroTrait::Prototype))
  {
    reset(HeroDebuff::Poisoned);
    reset(HeroDebuff::ManaBurned);
    reset(HeroStatus::Healthform);
    reset(HeroStatus::Manaform);
    stats.refresh();
  }
}

void Hero::addDodgeChancePercent(unsigned percent, bool isPermanent)
{
  const auto statusToUpdate = isPermanent ? HeroStatus::DodgePermanent : HeroStatus::DodgeTemporary;
  const auto newDodgeChance = std::min(getIntensity(statusToUpdate) + percent, 100u);
  setStatusIntensity(statusToUpdate, newDodgeChance);
}

unsigned Hero::getDodgeChancePercent() const
{
  const unsigned dodgeChance =
      std::min(getIntensity(HeroStatus::DodgePermanent) + getIntensity(HeroStatus::DodgeTemporary), 100u);
  if (has(HeroStatus::Pessimist) && dodgeChance != 100u)
    return 0;
  return dodgeChance;
}

bool Hero::predictDodgeNext() const
{
  assert(has(HeroStatus::DodgePrediction) && "predictDodgeNext called without dodge prediction status");
  if (has(HeroStatus::Pessimist))
    return false;
  return dodgeNext;
}

bool Hero::tryDodge(Monsters& allMonsters)
{
  const bool success = dodgeNext && (!has(HeroStatus::Pessimist) || getDodgeChancePercent() == 100);
  rerollDodgeNext();
  if (success)
  {
    reset(HeroStatus::DodgePrediction);
    reset(HeroStatus::DodgeTemporary);
    applyOrCollect(faith.dodgedAttack(), allMonsters);
  }
  return success;
}

void Hero::wallDestroyed()
{
  if (has(Boon::StoneForm) && !has(HeroStatus::Might))
    add(HeroStatus::Might);
  if (has(ShopItem::RockHeart))
  {
    healHitPoints(1);
    recoverManaPoints(1);
  }
  if (getFollowedDeity() == God::BinlorIronshield)
    applyOrCollectPietyGain(5);
}

void Hero::plantDestroyed(bool wasPhysicalAttack)
{
  if (wasPhysicalAttack)
    removeOneTimeAttackEffects();
  Monsters ignore;
  applyOrCollect(faith.plantDestroyed(), ignore);
}

bool Hero::bloodPoolConsumed()
{
  const auto sanguine = getIntensity(HeroStatus::Sanguine);
  if (sanguine == 0)
    return false;
  healHitPoints(getHitPointsMax() * sanguine / 100, false);
  Monsters ignore;
  applyOrCollect(faith.bloodPoolConsumed(receivedBoonCount(Boon::BloodTithe)), ignore);
  if (has(HeroTrait::Insane))
    recoverManaPoints(1);
  return true;
}

bool Hero::petrifyPlant(Monsters& allMonsters)
{
  if (!has(Spell::Imawal))
    return false;
  const auto manaCosts = Magic::spellCosts(Spell::Imawal, *this);
  if (getManaPoints() < manaCosts)
    return false;
  loseManaPoints(manaCosts);
  applyOrCollect(faith.imawalPetrifyPlant(manaCosts), allMonsters);
  return true;
}

void Hero::rerollDodgeNext()
{
  std::uniform_int_distribution<unsigned> number(1, 100);
  dodgeNext = getDodgeChancePercent() >= number(generator);
}

void Hero::applyDragonSoul(unsigned manaCosts)
{
  if (has(HeroStatus::Pessimist))
    return;
  std::uniform_int_distribution<> number(1, 100);
  if (number(generator) <= 15)
    recoverManaPoints(manaCosts);
}

void Hero::chargeFireHeart()
{
  inventory.chargeFireHeart();
}

void Hero::chargeCrystalBall()
{
  inventory.chargeCrystalBall();
}

unsigned Hero::gold() const
{
  return inventory.gold;
}

void Hero::addGold(unsigned amountReceived)
{
  inventory.gold += amountReceived;
}

bool Hero::spendGold(unsigned amountSpent)
{
  if (inventory.gold < amountSpent)
    return false;
  inventory.gold -= amountSpent;
  return true;
}

void Hero::collectGoldPile()
{
  unsigned amount = std::uniform_int_distribution<unsigned>(1, 3)(generator);
  if (has(HeroTrait::BlackMarket))
    ++amount;
  inventory.gold += amount;
}

int Hero::buyingPrice(Item item) const
{
  return inventory.buyingPrice(item);
}

int Hero::sellingPrice(Item item) const
{
  return inventory.sellingPrice(item);
}

bool Hero::canBuy(Item item) const
{
  return hasRoomFor(item) && canAfford(item);
}

bool Hero::buy(Item item)
{
  const int price = buyingPrice(item);
  if (price < 0 || !hasRoomFor(item) || !spendGold(static_cast<unsigned>(price)))
    return false;
  if (has(HeroTrait::RegalPerks))
    healHitPoints(getHitPointsMax() / 2, true);
  if (!receive(item))
    assert(false);
  return true;
}

Faith& Hero::getFaith()
{
  return faith;
}

const Faith& Hero::getFaith() const
{
  return faith;
}

unsigned Hero::getPiety() const
{
  return faith.getPiety();
}

std::optional<God> Hero::getFollowedDeity() const
{
  return faith.getFollowedDeity();
}

bool Hero::has(Boon boon) const
{
  return faith.has(boon);
}

unsigned Hero::receivedBoonCount(Boon boon) const
{
  return faith.boonCount(boon);
}

int Hero::getBoonCosts(Boon boon) const
{
  return faith.getCosts(boon, *this);
}

bool Hero::followDeity(God god, unsigned numRevealedTiles, Resources& resources)
{
  return faith.followDeity(god, *this, numRevealedTiles, resources);
}

bool Hero::request(BoonOrPact boonOrPact, Monsters& allMonsters, Resources& resources)
{
  if (const auto boon = std::get_if<Boon>(&boonOrPact))
    return faith.request(*boon, *this, allMonsters, resources);
  return faith.enter(std::get<Pact>(boonOrPact));
}

bool Hero::desecrate(God altar, Monsters& allMonsters)
{
  return faith.desecrate(altar, *this, allMonsters, has(ShopItem::AgnosticCollar));
}

void Hero::startPietyCollection()
{
  assert(!collectedPiety);
  collectedPiety.emplace();
}

void Hero::collect(PietyChange pietyChange)
{
  assert(collectedPiety);
  *collectedPiety += pietyChange;
}

void Hero::applyCollectedPiety(Monsters& allMonsters)
{
  assert(collectedPiety);
  faith.apply(*collectedPiety, *this, allMonsters);
  collectedPiety.reset();
}

void Hero::resetCollectedPiety()
{
  collectedPiety.reset();
}

void Hero::applyOrCollect(PietyChange pietyChange, Monsters& allMonsters)
{
  if (collectedPiety)
    *collectedPiety += pietyChange;
  else
    faith.apply(pietyChange, *this, allMonsters);
}

void Hero::applyOrCollectPietyGain(unsigned pointsGained)
{
  if (collectedPiety)
    *collectedPiety += static_cast<int>(pointsGained);
  else
    faith.gainPiety(pointsGained);
}

void Hero::setHitPointsMax(unsigned hitPointsMax)
{
  stats.setHitPointsMax(HitPoints{hitPointsMax});
}

void Hero::setManaPointsMax(unsigned manaPointsMax)
{
  stats.setManaPointsMax(ManaPoints{manaPointsMax});
}

void Hero::changeHitPointsMax(int deltaPoints)
{
  // TODO: Add tests for protection against underflow in all applicable change* methods
  const auto newMax = changeHelper(stats.getHitPointsMax(), deltaPoints, 1_HP);
  stats.setHitPointsMax(newMax);
}

void Hero::changeManaPointsMax(int deltaPoints)
{
  const auto newMax = changeHelper(stats.getManaPointsMax(), deltaPoints, 0_MP);
  stats.setManaPointsMax(newMax);
}

void Hero::changePhysicalResistPercentMax(int deltaPoints)
{
  defence.changePhysicalResistPercentMax(deltaPoints);
}

void Hero::changeMagicalResistPercentMax(int deltaPoints)
{
  defence.changeMagicalResistPercentMax(deltaPoints);
}

void Hero::modifyLevelBy(int delta)
{
  experience.modifyLevelBy(delta);
}

void Hero::addConversionPoints(unsigned points, Monsters& allMonsters)
{
  if (conversion.addPoints(points))
    conversion.applyBonus(*this, allMonsters);
}

bool Hero::lose(Item item)
{
  const bool itemLost = inventory.remove(item);
  if (itemLost)
    changeStatsFromItem(item, false);
  return itemLost;
}

bool Hero::receiveFreeSpell(Spell spell)
{
  return inventory.addFree(spell);
}

void Hero::receiveEnlightenment(Monsters& allMonsters)
{
  const auto enchantedBeads = inventory.enchantPrayerBeads();
  changeHitPointsMax(static_cast<int>(enchantedBeads));
  changeDamageBonusPercent(static_cast<int>(enchantedBeads));
  changeManaPointsMax(+5);
  if (conversion.addPoints(10 * enchantedBeads))
    conversion.applyBonus(*this, allMonsters);
  gainExperienceNoBonuses(enchantedBeads, allMonsters);
  reset(HeroDebuff::Cursed);
}

void Hero::clearInventory()
{
  for (auto& entry : inventory.getItemsAndSpells())
  {
    if (auto item = std::get_if<Item>(&entry.itemOrSpell))
      changeStatsFromItem(*item, false);
  }
  inventory.clear();
}

const std::vector<Inventory::Entry>& Hero::getItemsAndSpells() const
{
  return inventory.getItemsAndSpells();
}

std::vector<std::pair<Item, unsigned>> Hero::getItemCounts() const
{
  return inventory.getItemCounts();
}

std::vector<std::pair<Spell, unsigned>> Hero::getSpellCounts() const
{
  return inventory.getSpellCounts();
}

std::vector<std::pair<Inventory::Entry, unsigned>> Hero::getItemsGrouped() const
{
  return inventory.getItemsGrouped();
}

std::vector<Inventory::Entry> Hero::getSpells() const
{
  return inventory.getSpells();
}

bool Hero::has(ItemOrSpell itemOrSpell) const
{
  return inventory.has(itemOrSpell);
}

bool Hero::hasRoomFor(ItemOrSpell itemOrSpell) const
{
  return inventory.hasRoomFor(itemOrSpell);
}

unsigned Hero::numFreeSmallInventorySlots() const
{
  return inventory.numFreeSmallSlots();
}

bool Hero::canAfford(Item item) const
{
  // Negative price means item cannot be traded
  const int price = buyingPrice(item);
  return price >= 0 && gold() >= static_cast<unsigned>(price);
}

bool Hero::receive(ItemOrSpell itemOrSpell)
{
  // TODO: Remove this workaround for "corroded" items (=items placed on a tile with a corrosive patch)
  const bool wasCorroded = [&itemOrSpell] {
    if (itemOrSpell == ItemOrSpell{MiscItem::WispGemCorroded})
      itemOrSpell = MiscItem::WispGem;
    else if (itemOrSpell == ItemOrSpell{MiscItem::WallCruncherCorroded})
      itemOrSpell = MiscItem::WallCruncher;
    else if (itemOrSpell == ItemOrSpell{MiscItem::CharmCorroded})
      itemOrSpell = MiscItem::Charm;
    else if (itemOrSpell == ItemOrSpell{Potion::CourageJuiceCorroded})
      itemOrSpell = Potion::CourageJuice;
    else
      return false;
    return true;
  }();
  if (wasCorroded)
  {
    std::vector<Monster> ignore;
    add(HeroDebuff::Corroded, ignore);
  }

  if (!inventory.add(itemOrSpell))
    return false;
  if (const auto item = std::get_if<Item>(&itemOrSpell))
    changeStatsFromItem(*item, true);
  return true;
}

void Hero::convert(ItemOrSpell itemOrSpell, Monsters& allMonsters)
{
  const auto conversionResult = inventory.removeForConversion(itemOrSpell);
  if (conversionResult)
  {
    const auto item = std::get_if<Item>(&itemOrSpell);
    if (item)
      changeStatsFromItem(*item, false);
    else if (has(Boon::Refreshment))
      recoverManaPoints(getManaPointsMax() * (faith.getFollowedDeity() == God::MysteraAnnur ? 50 : 25) / 100);

    const auto [conversionPoints, wasSmall] = *conversionResult;
    if (conversion.addPoints(conversionPoints))
      conversion.applyBonus(*this, allMonsters);

    applyOrCollect(faith.converted(itemOrSpell, wasSmall), allMonsters);
    if (item && std::holds_alternative<TaurogItem>(*item))
      faith.convertedTaurogItem(*this, allMonsters);
  }
}

bool Hero::canConvert(ItemOrSpell itemOrSpell) const
{
  return inventory.canConvert(itemOrSpell);
}

bool Hero::canUse(ShopItem item) const
{
  switch (item)
  {
  // Basic Items
  case ShopItem::BadgeOfHonour:
    return !has(HeroStatus::DeathProtection);

  // Quest Items
  case ShopItem::FireHeart:
    return inventory.getFireHeartCharge() > 0;
  case ShopItem::CrystalBall:
    return inventory.getCrystalBallCharge() > 0 && gold() >= inventory.getCrystalBallUseCosts();

  // Elite Items
  case ShopItem::KegOfHealth:
  case ShopItem::KegOfMana:
  case ShopItem::AmuletOfYendor:
    return true;

  case ShopItem::OrbOfZot:
  case ShopItem::WickedGuitar:
    return true;

  default:
    return false;
  }
}

bool Hero::canUse(BossReward item) const
{
  return item == BossReward::NamtarsWard && !has(HeroStatus::DeathProtection) && !namtarsWardUsedThisLevel;
}

void Hero::use(Potion potion, Monsters& allMonsters)
{
  if (has(HeroTrait::Colourants))
  {
    reset(HeroStatus::Healthform);
    reset(HeroStatus::Manaform);
  }

  switch (potion)
  {
  case Potion::HealthPotion:
    drinkHealthPotion();
    break;
  case Potion::ManaPotion:
    drinkManaPotion();
    break;
  case Potion::FortitudeTonic:
    reset(HeroDebuff::Poisoned);
    reset(HeroDebuff::Weakened);
    break;
  case Potion::BurnSalve:
    reset(HeroDebuff::ManaBurned);
    reset(HeroDebuff::Corroded);
    break;
  case Potion::StrengthPotion:
    addSpiritStrength();
    break;
  case Potion::Schadenfreude:
    add(HeroStatus::Schadenfreude);
    break;
  case Potion::QuicksilverPotion:
    add(HeroStatus::DodgePrediction);
    add(HeroStatus::DodgeTemporary, 50);
    break;
  case Potion::ReflexPotion:
    add(HeroStatus::Reflexes);
    break;
  case Potion::CanOfWhupaz:
    add(HeroStatus::CrushingBlow);
    break;
  case Potion::CourageJuice:
  case Potion::CourageJuiceCorroded:
    add(HeroStatus::Might);
    add(HeroStatus::FirstStrikeTemporary);
    add(HeroStatus::ExperienceBoost);
    stats.loseHitPointsWithoutDeathProtection(stats.getHitPoints() - 1_HP);
    break;
  }
  if (has(ShopItem::Trisword))
    changeBaseDamage(inventory.chargeTrisword());
  if (has(ShopItem::AlchemistScroll) && !alchemistScrollUsedThisLevel && spendGold(3))
  {
    changeHitPointsMax(8);
    alchemistScrollUsedThisLevel = true;
  }
  inventory.remove(potion);
  applyOrCollect(faith.drankPotion(potion), allMonsters);
}

void Hero::use(ShopItem item, Monsters& allMonsters)
{
  bool consumed = false;
  switch (item)
  {
  // Basic Items
  case ShopItem::BadgeOfHonour:
    if (!has(HeroStatus::DeathProtection))
    {
      add(HeroStatus::DeathProtection);
      consumed = true;
    }
    break;

  // Quest Items
  case ShopItem::FireHeart:
    healHitPoints(getHitPointsMax() * inventory.fireHeartUsed() / 100);
    break;
  case ShopItem::CrystalBall:
    if (spendGold(inventory.getCrystalBallUseCosts()))
      recoverManaPoints(inventory.crystalBallUsed());
    break;

  // Elite Items
  case ShopItem::KegOfHealth:
  {
    [[maybe_unused]] const bool ok = inventory.remove(ShopItem::KegOfHealth) && inventory.add(Potion::HealthPotion) &&
                                     inventory.add(Potion::HealthPotion) && inventory.add(Potion::HealthPotion);
    assert(ok);
    break;
  }
  case ShopItem::KegOfMana:
  {
    [[maybe_unused]] const bool ok = inventory.remove(ShopItem::KegOfMana) && inventory.add(Potion::ManaPotion) &&
                                     inventory.add(Potion::ManaPotion) && inventory.add(Potion::ManaPotion);
    assert(ok);
    break;
  }
  case ShopItem::AmuletOfYendor:
    gainExperienceNoBonuses(50, allMonsters);
    consumed = true;
    break;

  case ShopItem::OrbOfZot:
    // TODO: Only apply to visible monsters
    for (auto& monster : allMonsters)
      monster.zot();
    consumed = true;
    break;
  case ShopItem::WickedGuitar:
    // TODO: Only apply to visible monsters
    for (auto& monster : allMonsters)
      monster.makeWickedSick();
    break;

  default:
    break;
  }
  if (consumed && inventory.remove(item))
    changeStatsFromItem(item, false);
}

void Hero::use(BossReward item)
{
  if (item == BossReward::NamtarsWard)
  {
    add(HeroStatus::DeathProtection);
    namtarsWardUsedThisLevel = true;
  }
}

bool Hero::canUse(Item item, const Monster& monster) const
{
  if (item == Item{BlacksmithItem::SlayerWand})
    return !monster.isDefeated() && monster.getLevel() < 10;
  return canUse(item);
}

void Hero::use(Item item, Monster& monster, Monsters& allMonsters)
{
  if (item == Item{BlacksmithItem::SlayerWand})
  {
    if (monster.grantsXP())
      gainExperienceForKill(std::min(getLevel(), monster.getLevel()), monster.isSlowed(), allMonsters);
    monster.die();
    inventory.remove(item);
  }
}

unsigned Hero::getFoodCount() const
{
  return inventory.getFoodCount();
}

bool Hero::useCompressionSealOn(ItemOrSpell itemOrSpell)
{
  if (has(AlchemistSeal::CompressionSeal) && inventory.compress(itemOrSpell))
  {
    lose(AlchemistSeal::CompressionSeal);
    return true;
  }
  return false;
}

bool Hero::useTransmutationSealOn(ItemOrSpell itemOrSpell, Monsters& allMonsters)
{
  if (has(AlchemistSeal::TransmutationSeal) && inventory.transmute(itemOrSpell))
  {
    inventory.remove(AlchemistSeal::TransmutationSeal);
    if (const auto item = std::get_if<Item>(&itemOrSpell))
    {
      changeStatsFromItem(*item, false);
      // Taurog doesn't tolerate transmuting one of his boons
      if (std::holds_alternative<TaurogItem>(*item))
        faith.convertedTaurogItem(*this, allMonsters);
    }
    return true;
  }
  return false;
}

bool Hero::useTransmutationSealOnPetrifiedEnemy()
{
  // A pile of gold will appear below an enemy petrified with Imawal (not with Death Gaze!).
  // Since we don't model gold piles yet, just add the gold directly.
  if (useTransmutationSealOnWallOrPetrifiedPlant())
  {
    collectGoldPile();
    return true;
  }
  return false;
}

bool Hero::useTransmutationSealOnWallOrPetrifiedPlant()
{
  if (has(AlchemistSeal::TransmutationSeal))
  {
    lose(AlchemistSeal::TransmutationSeal);
    addGold(has(HeroTrait::BlackMarket) ? 11 : 10);
    return true;
  }
  return false;
}

bool Hero::useTranslocationSealOn(Item shopItem)
{
  if (has(AlchemistSeal::TranslocationSeal))
  {
    if (!isSmall(shopItem) && inventory.numFreeSmallSlots() < 4 && !has(HeroTrait::RegalSize))
      return false;
    lose(AlchemistSeal::TranslocationSeal);
    return inventory.translocate(shopItem);
  }
  return false;
}

// Methods that add or remove passive item effects on hero
void Hero::changeStatsImpl(BlacksmithItem item, bool itemReceived)
{
  const int sign = itemReceived ? +1 : -1;
  switch (item)
  {
  case BlacksmithItem::PerseveranceBadge:
    changeDamageBonusPercent(10 * sign);
    break;
  case BlacksmithItem::ReallyBigSword:
    add(HeroStatus::PiercePhysical, sign);
    add(HeroStatus::SlowStrike, sign);
    break;
  case BlacksmithItem::BearMace:
    add(HeroStatus::Knockback, 25 * sign);
    break;
  case BlacksmithItem::Sword:
    changeBaseDamage(2 * sign);
    break;
  case BlacksmithItem::Shield:
    add(HeroStatus::DamageReduction, 2 * sign);
    break;
  case BlacksmithItem::SlayerWand:
    break;
  }
}

void Hero::changeStatsImpl(ShopItem item, bool itemReceived)
{
  const int sign = itemReceived ? +1 : -1;
  switch (item)
  {
  case ShopItem::BadgeOfHonour:
    changeDamageBonusPercent(10 * sign);
    break;
  case ShopItem::BloodySigil:
    changeHitPointsMax(5 * sign);
    changeDamageBonusPercent(-10 * sign);
    break;
  case ShopItem::FineSword:
    changeBaseDamage(4 * sign);
    break;
  case ShopItem::PendantOfHealth:
    changeHitPointsMax(10 * sign);
    break;
  case ShopItem::PendantOfMana:
    changeManaPointsMax(2 * sign);
    break;
  case ShopItem::Spoon:
    changeBaseDamage(sign);
    break;
  case ShopItem::TowerShield:
    changePhysicalResistPercent(10 * sign);
    break;
  case ShopItem::TrollHeart:
    modifyFutureHealthBonus(2 * sign);
    break;
  case ShopItem::HerosHelm:
    changeHitPointsMax(5 * sign);
    changeManaPointsMax(sign);
    changeBaseDamage(2 * sign);
    break;
  case ShopItem::Platemail:
    add(HeroStatus::DamageReduction, 2 * sign * static_cast<int>(getLevel()));
    add(HeroStatus::SlowStrike, sign);
    break;
  case ShopItem::Whurrgarbl:
    add(HeroStatus::BurningStrike, sign);
    break;
  case ShopItem::Trisword:
    changeBaseDamage(sign * inventory.getTriswordDamage());
    break;
  case ShopItem::VenomDagger:
    if (sign > 0 || getIntensity(HeroStatus::Poisonous) > 2)
      add(HeroStatus::Poisonous, 2 * sign);
    else
      // We can end up here if Tikki Tooki's punishment was triggered
      reset(HeroStatus::Poisonous);
    break;
  case ShopItem::MartyrWraps:
    add(HeroStatus::CorrosiveStrike, sign);
    break;
  case ShopItem::MagePlate:
  {
    const int strength = (getLevel() + 1) / 2;
    changeManaPointsMax(sign * strength);
    changeDamageBonusPercent(-5 * sign * strength);
    break;
  }
  case ShopItem::VampiricBlade:
    add(HeroStatus::LifeSteal, sign);
    break;
  case ShopItem::ViperWard:
    if (itemReceived)
      add(HeroStatus::PoisonImmune);
    else if (!has(HeroTrait::Scars) && !has(HeroTrait::Undead))
      reset(HeroStatus::PoisonImmune);
    break;
  case ShopItem::SoulOrb:
    if (itemReceived)
      add(HeroStatus::ManaBurnImmune);
    else if (!has(HeroTrait::Scars) && !has(HeroTrait::Undead))
      reset(HeroStatus::ManaBurnImmune);
    break;
  case ShopItem::ElvenBoots:
    changeManaPointsMax(3 * sign);
    changeMagicalResistPercent(15 * sign);
    break;
  case ShopItem::DwarvenGauntlets:
    changeDamageBonusPercent(20 * sign);
    if (itemReceived)
      modifyFutureHealthBonus(2 * sign);
    break;
  case ShopItem::OrbOfZot:
    changeHitPointsMax(5 * sign);
    changeManaPointsMax(3 * sign);
    break;
  default:
    break;
  }
}

void Hero::changeStatsImpl(MiscItem item, bool itemReceived)
{
  if (item == MiscItem::Gorgward)
  {
    if (itemReceived)
      add(HeroStatus::DeathGazeImmune);
    else if (!has(HeroTrait::AzureBody))
      reset(HeroStatus::DeathGazeImmune);
  }
  else if (item == MiscItem::Charm)
  {
    stats.setBaseDamage(itemReceived ? stats.getBaseDamage() + 1_damage : stats.getBaseDamage() - 1_damage);
    stats.setHitPointsMax(itemReceived ? stats.getHitPointsMax() + 1_HP : stats.getHitPointsMax() - 1_HP);
  }
  else if (item == MiscItem::WispGem)
  {
    stats.setDamageBonus(stats.getDamageBonus() + DamageBonus{itemReceived ? +5 : -5});
  }
}

void Hero::changeStatsImpl(BossReward item, bool itemReceived)
{
  const int sign = itemReceived ? +1 : -1;
  if (item == BossReward::DragonShield)
  {
    changePhysicalResistPercent(18 * sign);
    changeMagicalResistPercent(18 * sign);
  }
  else if (item == BossReward::AvatarsCodex)
    add(HeroStatus::HeavyFireball, sign);
}

void Hero::changeStatsImpl(TaurogItem item, bool itemReceived)
{
  switch (item)
  {
  case TaurogItem::Skullpicker:
    changeBaseDamage(itemReceived ? +5 : -5);
    break;
  case TaurogItem::Wereward:
    add(HeroStatus::DamageReduction, itemReceived ? +5 : -5);
    break;
  case TaurogItem::Gloat:
    changeMagicalResistPercent(itemReceived ? +15 : -15);
    break;
  case TaurogItem::Will:
    changePhysicalResistPercent(itemReceived ? +15 : -15);
    break;
  }
}

void Hero::changeStatsFromItem(Item item, bool itemReceived)
{
  std::visit(overloaded{[](const Potion&) {}, [](const AlchemistSeal&) {},
                        [&](const auto& item) { changeStatsImpl(item, itemReceived); }},
             item);
}

uint8_t Hero::getConversionPoints() const
{
  return conversion.getPoints();
}

uint8_t Hero::getConversionThreshold() const
{
  return conversion.getThreshold();
}

using namespace std::string_literals;

std::vector<std::string> describe(const Hero& hero)
{
  if (hero.isDefeated())
    return {"Hero defeated."};

  std::vector description{
      hero.getName() + " level " + std::to_string(hero.getLevel()),
      std::to_string(hero.getHitPoints()) + "/" + std::to_string(hero.getHitPointsMax()) + " HP",
      std::to_string(hero.getManaPoints()) + "/" + std::to_string(hero.getManaPointsMax()) + " MP",
      std::to_string(hero.getXP()) + "/" + std::to_string(hero.getXPforNextLevel()) + " XP",
      std::to_string(hero.getConversionPoints()) + "/" + std::to_string(hero.getConversionThreshold()) + " CP",
      std::to_string(hero.getDamageVersusStandard()) + " damage (" + std::to_string(hero.getBaseDamage()) +
          (hero.getDamageBonusPercent() >= 0 ? "+" : "") + std::to_string(hero.getDamageBonusPercent()) + "%)",
      std::to_string(hero.getPiety()) + " piety",
      std::to_string(hero.gold()) + " gold"};
  if (hero.has(HeroTrait::Herbivore))
    description.emplace_back(std::to_string(hero.getFoodCount()) + " food");
  if (hero.getPhysicalResistPercent() > 0)
    description.emplace_back(std::to_string(hero.getPhysicalResistPercent()) + "% physical resist");
  if (hero.getMagicalResistPercent() > 0)
    description.emplace_back(std::to_string(hero.getMagicalResistPercent()) + "% magic resist");

  for (int i = 0; i < static_cast<int>(HeroDebuff::Last); ++i)
  {
    const auto debuff = static_cast<HeroDebuff>(i);
    const auto intensity = hero.getIntensity(debuff);
    if (intensity > 0)
    {
      if (canHaveMultiple(debuff) && intensity > 1)
        description.emplace_back("is "s + toString(debuff) + " (x" + std::to_string(intensity) + ")");
      else
        description.emplace_back("is "s + toString(debuff));
    }
  }

  const bool isPessimist = hero.has(HeroStatus::Pessimist);
  for (int i = 0; i < static_cast<int>(HeroStatus::Last); ++i)
  {
    const auto status = static_cast<HeroStatus>(i);
    const auto intensity = hero.getIntensity(status);
    if (intensity == 0 ||
        (isPessimist && (status == HeroStatus::DodgePermanent || status == HeroStatus::DodgeTemporary ||
                         status == HeroStatus::DodgePrediction)))
      continue;
    if (status == HeroStatus::DodgePrediction)
    {
      if (hero.predictDodgeNext())
        description.emplace_back("will dodge next attack");
      else
        description.emplace_back("won't dodge next attack");
      continue;
    }
    const bool useIs = status == HeroStatus::CurseImmune || status == HeroStatus::DeathGazeImmune ||
                       status == HeroStatus::Exhausted || status == HeroStatus::ManaBurnImmune ||
                       status == HeroStatus::Poisonous || status == HeroStatus::PoisonImmune;
    std::string statusStr;
    if (useIs)
      statusStr = "is ";
    else
      statusStr = "has ";
    statusStr += toString(status);
    if (canHaveMultiple(status))
    {
      if (intensity > 1)
        statusStr += " (x" + std::to_string(intensity) + ")";
    }
    description.emplace_back(std::move(statusStr));
  }

  if (hero.getFollowedDeity())
    description.emplace_back("follows "s + toString(*hero.getFollowedDeity()));
  const auto indulgence = hero.getFaith().getIndulgence();
  if (indulgence > 0)
    description.emplace_back("indulgence points: "s + std::to_string(indulgence));
  for (auto boon :
       {Boon::StoneForm, Boon::BloodCurse, Boon::Humility, Boon::Petition, Boon::Flames, Boon::MysticBalance})
  {
    if (hero.has(boon))
      description.emplace_back("has "s + toString(boon));
  }
  const auto pact = hero.getFaith().getPact();
  if (pact)
    description.emplace_back("entered "s + toString(*pact));
  if (hero.getFaith().enteredConsensus())
    description.emplace_back("reached consensus");

  return description;
}

std::vector<std::string> describe_diff(const Hero& before, const Hero& now)
{
  if (now.isDefeated())
    return {"Hero defeated."};

  std::vector<std::string> description;
  if (before.getLevel() != now.getLevel())
    description.emplace_back("level "s + std::to_string(before.getLevel()) + " -> " + std::to_string(now.getLevel()));
  if (before.getHitPoints() != now.getHitPoints() || before.getHitPointsMax() != now.getHitPointsMax())
    description.emplace_back("HP: "s + std::to_string(before.getHitPoints()) + "/" +
                             std::to_string(before.getHitPointsMax()) + " -> " + std::to_string(now.getHitPoints()) +
                             "/" + std::to_string(now.getHitPointsMax()));
  if (before.getManaPoints() != now.getManaPoints() || before.getManaPointsMax() != now.getManaPointsMax())
    description.emplace_back("MP: "s + std::to_string(before.getManaPoints()) + "/" +
                             std::to_string(before.getManaPointsMax()) + " -> " + std::to_string(now.getManaPoints()) +
                             "/" + std::to_string(now.getManaPointsMax()));
  if (before.getXP() != now.getXP() || before.getXPforNextLevel() != now.getXPforNextLevel())
    description.emplace_back("XP: "s + std::to_string(before.getXP()) + "/" +
                             std::to_string(before.getXPforNextLevel()) + " -> " + std::to_string(now.getXP()) + "/" +
                             std::to_string(now.getXPforNextLevel()));
  if (before.getConversionPoints() != now.getConversionPoints() ||
      before.getConversionThreshold() != now.getConversionThreshold())
    description.emplace_back(
        "CP: "s + std::to_string(before.getConversionPoints()) + "/" + std::to_string(before.getConversionThreshold()) +
        " -> " + std::to_string(now.getConversionPoints()) + "/" + std::to_string(now.getConversionThreshold()));
  if (before.getDamageVersusStandard() != now.getDamageVersusStandard())
    description.emplace_back(
        "damage: "s + std::to_string(before.getDamageVersusStandard()) + " (" + std::to_string(before.getBaseDamage()) +
        "+" + std::to_string(before.getDamageBonusPercent()) + "%) -> " +
        std::to_string(now.getDamageVersusStandard()) + " (" + std::to_string(now.getBaseDamage()) + "+" +
        std::to_string(now.getDamageBonusPercent()) + "%)");
  if (before.getPiety() != now.getPiety())
    description.emplace_back("piety: "s + std::to_string(before.getPiety()) + " -> " + std::to_string(now.getPiety()));
  if (before.gold() != now.gold())
    description.emplace_back("gold: " + std::to_string(before.gold()) + " -> " + std::to_string(now.gold()));

  if (before.getPhysicalResistPercent() != now.getPhysicalResistPercent())
    description.emplace_back("physical resist: "s + std::to_string(before.getPhysicalResistPercent()) + "% -> " +
                             std::to_string(now.getPhysicalResistPercent()) + "%");
  if (before.getMagicalResistPercent() != now.getMagicalResistPercent())
    description.emplace_back("magical resist: "s + std::to_string(before.getMagicalResistPercent()) + "% -> " +
                             std::to_string(now.getMagicalResistPercent()) + "%");

  for (int i = 0; i < static_cast<int>(HeroDebuff::Last); ++i)
  {
    const auto debuff = static_cast<HeroDebuff>(i);
    const auto intensityBefore = before.getIntensity(debuff);
    const auto intensityNow = now.getIntensity(debuff);
    if (intensityBefore != intensityNow)
    {
      if (canHaveMultiple(debuff))
      {
        description.emplace_back(toString(debuff) + " x"s + std::to_string(intensityBefore) + " -> x" +
                                 std::to_string(intensityNow));
      }
      else if (intensityBefore == 0)
        description.emplace_back("is "s + toString(debuff));
      else
        description.emplace_back("was "s + toString(debuff));
    }
  }

  for (int i = 0; i < static_cast<int>(HeroStatus::Last); ++i)
  {
    const auto status = static_cast<HeroStatus>(i);
    const auto intensityBefore = before.getIntensity(status);
    const auto intensityNow = now.getIntensity(status);
    if (intensityBefore == intensityNow)
      continue;
    if ((now.has(HeroStatus::Pessimist) &&
         (status == HeroStatus::DodgePermanent || status == HeroStatus::DodgeTemporary ||
          status == HeroStatus::DodgePrediction)))
      continue;
    if (canHaveMultiple(status))
    {
      description.emplace_back(toString(status) + " x"s + std::to_string(intensityBefore) + " -> x" +
                               std::to_string(intensityNow));
    }
    else
    {
      const bool useIs = status == HeroStatus::CurseImmune || status == HeroStatus::DeathGazeImmune ||
                         status == HeroStatus::Exhausted || status == HeroStatus::ManaBurnImmune ||
                         status == HeroStatus::Poisonous || status == HeroStatus::PoisonImmune;
      const bool usePast = !now.has(status);
      std::string statusStr = [useIs, usePast]() -> std::string {
        if (useIs)
        {
          if (usePast)
            return "was ";
          else
            return "is ";
        }
        if (usePast)
          return "lost ";
        return "has ";
      }() + toString(status);
      description.emplace_back(std::move(statusStr));
    }
  }

  if (before.getFaith().getIndulgence() != now.getFaith().getIndulgence())
    description.emplace_back("indulgence: "s + std::to_string(before.getFaith().getIndulgence()) + " -> " +
                             std::to_string(now.getFaith().getIndulgence()));

  return description;
}
