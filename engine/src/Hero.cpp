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
  : Hero(DungeonSetup{heroClass, heroRace})
{
}

Hero::Hero(const DungeonSetup& setup)
  : name(isMonsterClass(setup.heroClass) ? toString(setup.heroRace)
                                         : (toString(setup.heroRace) + std::string(" ") + toString(setup.heroClass)))
  , traits(startingTraits(setup.heroClass))
  , stats()
  , defence(0, 0, 65, 65)
  , experience()
  , inventory(setup)
  , conversion(setup)
  , faith(setup.altar)
  , statuses()
  , collectedPiety()
  , generator(std::random_device{}())
{
  if (hasTrait(HeroTrait::Veteran))
    experience = Experience(Experience::IsVeteran{});
  if (hasTrait(HeroTrait::Dangerous))
    stats = HeroStats(HeroStats::IsDangerous{});
  if (hasTrait(HeroTrait::RegalSize))
    stats = HeroStats(HeroStats::RegalSize{});

  if (hasTrait(HeroTrait::PitDog))
    addStatus(HeroStatus::DeathProtection);
  if (hasTrait(HeroTrait::Mageslay))
    changeDamageBonusPercent(+20);
  if (hasTrait(HeroTrait::Spellkill))
    defence.setMagicalResistPercent(50);
  if (hasTrait(HeroTrait::ArcaneKnowledge))
    stats.setManaPointsMax(stats.getManaPointsMax() + 5);
  if (hasTrait(HeroTrait::Insane))
  {
    setStatusIntensity(HeroStatus::Sanguine, 20);
    stats.setManaPointsMax(stats.getManaPointsMax() - 3);
  }
  if (hasTrait(HeroTrait::Dexterous))
    addStatus(HeroStatus::FirstStrikePermanent);
  if (hasTrait(HeroTrait::Evasive))
    addDodgeChancePercent(20, true);
  if (hasTrait(HeroTrait::PoisonedBlade))
    changeDamageBonusPercent(-20);
  if (hasTrait(HeroTrait::GoodHealth))
  {
    stats.addHealthBonus(1);
    stats.addHealthBonus(1);
    stats.addHealthBonus(1);
  }
  if (hasTrait(HeroTrait::HandToHand))
  {
    stats.setBaseDamage(3);
    changeDamageBonusPercent(-30);
  }
  if (hasTrait(HeroTrait::DiamondBody))
  {
    defence.setPhysicalResistPercent(50);
    defence.setPhysicalResistPercentMax(75);
  }
  if (hasTrait(HeroTrait::HolyShield))
    defence.setPhysicalResistPercent(25);
  if (hasTrait(HeroTrait::Scars))
  {
    addStatus(HeroStatus::PoisonImmune);
    addStatus(HeroStatus::ManaBurnImmune);
    addStatus(HeroStatus::CurseImmune);
  }
  if (hasTrait(HeroTrait::Undead))
  {
    addStatus(HeroStatus::PoisonImmune);
    addStatus(HeroStatus::ManaBurnImmune);
  }
  if (hasTrait(HeroTrait::EternalThirst))
  {
    addStatus(HeroStatus::Sanguine, 5);
    addStatus(HeroStatus::LifeSteal, 1);
  }
  if (hasTrait(HeroTrait::RegalHygiene))
    addStatus(HeroStatus::CorrosiveStrike);

  // TODO: Move gold pile size to MapResources? (+ remove this trait)
  if (setup.modifiers.count(ThievesModifier::BlackMarket))
    addTrait(HeroTrait::BlackMarket);
  // TODO: Store prepared altar, if any
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

unsigned Hero::getXP() const
{
  return experience.getXP();
}

unsigned Hero::getLevel() const
{
  return experience.getLevel();
}

unsigned Hero::getPrestige() const
{
  return experience.getPrestige();
}

unsigned Hero::getXPforNextLevel() const
{
  return experience.getXPforNextLevel();
}

void Hero::gainExperienceForKill(unsigned monsterLevel, bool monsterWasSlowed, Monsters& allMonsters)
{
  const auto xpBase = Experience::forHeroAndMonsterLevels(getLevel(), monsterLevel);
  auto xpBonuses = getStatusIntensity(HeroStatus::Learning);
  if (monsterWasSlowed)
    ++xpBonuses;
  if (hasTrait(HeroTrait::Veteran))
    ++xpBonuses;
  if (has(ShopItem::BalancedDagger) && getLevel() == monsterLevel)
    xpBonuses += 2;
  gainExperience(xpBase, xpBonuses, allMonsters);
}

void Hero::gainExperienceForPetrification(bool monsterWasSlowed, Monsters& allMonsters)
{
  if (monsterWasSlowed)
    gainExperience(0, 1, allMonsters);
}

void Hero::gainExperienceNoBonuses(unsigned xpGained, Monsters& allMonsters)
{
  gainExperience(0, xpGained, allMonsters);
}

void Hero::gainExperience(unsigned xpBase, unsigned xpBonuses, Monsters& allMonsters)
{
  auto level = getLevel();
  const auto prestige = getPrestige();
  experience.gain(xpBase, xpBonuses, hasStatus(HeroStatus::ExperienceBoost));
  if (xpBase > 0)
    resetStatus(HeroStatus::ExperienceBoost);
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
  return stats.getHitPoints();
}

uint16_t Hero::getHitPointsMax() const
{
  return stats.getHitPointsMax();
}

uint16_t Hero::getManaPoints() const
{
  return stats.getManaPoints();
}

uint16_t Hero::getManaPointsMax() const
{
  return stats.getManaPointsMax();
}

void Hero::drinkHealthPotion()
{
  const bool hasNagaCauldron = has(BossReward::NagaCauldron);
  unsigned percentHealed = hasTrait(HeroTrait::GoodDrink) ? 100 : 40;
  if (hasNagaCauldron)
    percentHealed += nagaCauldronBonus();
  stats.healHitPoints(getHitPointsMax() * percentHealed / 100, hasNagaCauldron);
  resetStatus(HeroDebuff::Poisoned);
  if (hasTrait(HeroTrait::Survivor))
    stats.recoverManaPoints(getManaPointsMax() * 2 / 10);
}

void Hero::drinkManaPotion()
{
  const bool hasNagaCauldron = has(BossReward::NagaCauldron);
  unsigned percentRestored = 40;
  if (hasTrait(HeroTrait::PowerHungry))
  {
    percentRestored = 60;
    addStatus(HeroStatus::Sanguine, 2);
  }
  if (hasNagaCauldron)
    percentRestored += nagaCauldronBonus();
  stats.recoverManaPoints(getManaPointsMax() * percentRestored / 100);
  resetStatus(HeroDebuff::ManaBurned);
  if (hasTrait(HeroTrait::Courageous))
    addStatus(HeroStatus::Might);
  if (hasTrait(HeroTrait::Survivor))
    stats.healHitPoints(getHitPointsMax() * 2 / 10, false);
}

unsigned Hero::nagaCauldronBonus() const
{
  return 5u * (static_cast<unsigned>(hasStatus(HeroDebuff::Poisoned)) +
               static_cast<unsigned>(hasStatus(HeroDebuff::ManaBurned)) +
               static_cast<unsigned>(hasStatus(HeroDebuff::Corroded)) +
               static_cast<unsigned>(hasStatus(HeroDebuff::Weakened)) +
               static_cast<unsigned>(hasStatus(HeroDebuff::Cursed)));
}

uint16_t Hero::getBaseDamage() const
{
  const auto damage = stats.getBaseDamage() + getStatusIntensity(HeroStatus::SpiritStrength);
  const auto weakened = getStatusIntensity(HeroDebuff::Weakened);
  return damage > weakened ? damage - weakened : 0u;
}

void Hero::changeBaseDamage(int deltaDamagePoints)
{
  const auto newDamage = static_cast<int>(stats.getBaseDamage()) + deltaDamagePoints;
  stats.setBaseDamage(newDamage > 0 ? static_cast<unsigned>(newDamage) : 0u);
}

int Hero::getDamageBonusPercent() const
{
  auto bonus = stats.getDamageBonusPercent();
  if (hasStatus(HeroStatus::Might))
    bonus += 30;
  if (hasTrait(HeroTrait::Determined) && stats.getHitPoints() * 2 < stats.getHitPointsMax())
    bonus += 30;
  return bonus;
}

void Hero::changeDamageBonusPercent(int deltaDamageBonusPercent)
{
  stats.setDamageBonusPercent(static_cast<int>(stats.getDamageBonusPercent()) + deltaDamageBonusPercent);
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
  if (hasTrait(HeroTrait::Bloodlust) && monster.getLevel() > getLevel())
    damage += standardDamage * 2 / 10;
  if (hasTrait(HeroTrait::Stabber) && monster.getHitPoints() >= monster.getHitPointsMax())
    damage += standardDamage * 3 / 10;
  if (hasTrait(HeroTrait::PoisonedBlade) && monster.isPoisoned())
    damage += standardDamage * 4 / 10;
  if (hasTrait(HeroTrait::GoodGolly) && monster.has(MonsterTrait::Undead))
    damage += getBaseDamage();
  return damage;
}

void Hero::addAttackBonus()
{
  changeDamageBonusPercent(+10);
}

void Hero::addHealthBonus()
{
  stats.addHealthBonus(experience.getUnmodifiedLevel());
}

void Hero::addManaBonus()
{
  stats.setManaPointsMax(stats.getManaPointsMax() + 1);
}

void Hero::modifyFutureHealthBonus(int amount)
{
  for (int i = 0; i < amount; ++i)
    stats.addHealthBonus(0);
  for (int i = 0; i < -amount; ++i)
    stats.reduceHealthBonus();
}

int Hero::getPhysicalResistPercent() const
{
  return defence.getPhysicalResistPercent();
}

int Hero::getMagicalResistPercent() const
{
  return defence.getMagicalResistPercent();
}

void Hero::setPhysicalResistPercent(int physicalResistPercent)
{
  defence.setPhysicalResistPercent(clampedTo<uint8_t>(physicalResistPercent));
}

void Hero::setMagicalResistPercent(int magicalResistPercent)
{
  defence.setMagicalResistPercent(clampedTo<uint8_t>(magicalResistPercent));
}

void Hero::changePhysicalResistPercent(int deltaPercent)
{
  setPhysicalResistPercent(defence.getPhysicalResistPercent(true) + deltaPercent);
}

void Hero::changeMagicalResistPercent(int deltaPercent)
{
  setMagicalResistPercent(defence.getMagicalResistPercent(true) + deltaPercent);
}

bool Hero::doesMagicalDamage() const
{
  return hasStatus(HeroStatus::ConsecratedStrike) || hasStatus(HeroStatus::MagicalAttack);
}

DamageType Hero::damageType() const
{
  return doesMagicalDamage() ? DamageType::Magical
                             : hasStatus(HeroStatus::PiercePhysical) ? DamageType::Piercing : DamageType::Physical;
}

bool Hero::hasInitiativeVersus(const Monster& monster) const
{
  if (hasStatus(HeroStatus::Reflexes))
    return true;

  const bool firstStrike = hasStatus(HeroStatus::FirstStrikePermanent) || hasStatus(HeroStatus::FirstStrikeTemporary);
  const bool heroFast = firstStrike && !hasStatus(HeroStatus::SlowStrike);
  const bool monsterFast = monster.has(MonsterTrait::FirstStrike) && !monster.isSlowed();
  if (heroFast || monsterFast)
    return !monsterFast;

  const bool heroSlow = !firstStrike && hasStatus(HeroStatus::SlowStrike);
  const bool monsterSlow = monster.isSlowed();
  if (heroSlow || monsterSlow)
    return !heroSlow;

  return getLevel() > monster.getLevel();
}

unsigned Hero::predictDamageTaken(unsigned attackerDamageOutput, DamageType damageType) const
{
  const auto reduction = getStatusIntensity(HeroStatus::DamageReduction);
  if (reduction < attackerDamageOutput)
    return defence.predictDamageTaken(attackerDamageOutput - reduction, damageType, 0);
  else
    return 0;
}

void Hero::loseHitPoints(unsigned amountPointsLost, Monsters& allMonsters)
{
  stats.loseHitPointsWithoutDeathProtection(amountPointsLost);
  if (stats.getHitPoints() == 0 && hasStatus(HeroStatus::DeathProtection))
  {
    stats.barelySurvive();
    resetStatus(HeroStatus::DeathProtection);
    applyOrCollect(faith.deathProtectionTriggered(), allMonsters);
  }
}

bool Hero::takeDamage(unsigned attackerDamageOutput, DamageType damageType, Monsters& allMonsters)
{
  const auto damagePoints = predictDamageTaken(attackerDamageOutput, damageType);
  loseHitPoints(damagePoints, allMonsters);
  if (damagePoints > 0 && hasStatus(HeroStatus::Schadenfreude))
  {
    recoverManaPoints(damagePoints);
    resetStatus(HeroStatus::Schadenfreude);
  }
  resetStatus(HeroStatus::StoneSkin);
  return damagePoints > 0;
}

void Hero::recover(unsigned nSquares)
{
  const bool exhausted = hasStatus(HeroStatus::Exhausted);
  if (!hasStatus(HeroDebuff::Poisoned))
    stats.healHitPoints(nSquares * recoveryMultiplier(), false);
  if (!hasStatus(HeroDebuff::ManaBurned) && !exhausted)
    stats.recoverManaPoints(nSquares);
}

unsigned Hero::recoveryMultiplier() const
{
  auto multiplier = getLevel();
  if (hasTrait(HeroTrait::Discipline))
    multiplier *= 2;
  if (has(ShopItem::BloodySigil))
    multiplier += 1;
  if (hasTrait(HeroTrait::Damned))
    multiplier = 1;
  return multiplier;
}

unsigned Hero::numSquaresForFullRecovery() const
{
  // TODO: For Goatperson, never return a number larger than the amount of food in inventory!

  auto numHP = 0u;
  if (!hasStatus(HeroDebuff::Poisoned))
  {
    const auto multiplier = recoveryMultiplier();
    numHP = (getHitPointsMax() - getHitPoints() + (multiplier - 1) /* always round up */) / multiplier;
  }
  const auto numMP = hasStatus(HeroDebuff::ManaBurned) ? 0u : getManaPointsMax() - getManaPoints();
  if (hasTrait(HeroTrait::Damned))
    return numHP + numMP;
  return std::max(numHP, numMP);
}

void Hero::healHitPoints(unsigned amountPointsHealed, bool mayOverheal)
{
  stats.healHitPoints(amountPointsHealed, mayOverheal);
}

void Hero::loseHitPointsOutsideOfFight(unsigned amountPointsLost, Monsters& allMonsters)
{
  loseHitPoints(amountPointsLost, allMonsters);
}

void Hero::recoverManaPoints(unsigned amountPointsRecovered)
{
  stats.recoverManaPoints(amountPointsRecovered);
}

void Hero::loseManaPoints(unsigned amountPointsLost)
{
  stats.loseManaPoints(amountPointsLost);
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

void Hero::addStatus(HeroStatus status, int addedIntensity)
{
  const int newIntensity = static_cast<int>(statuses[status]) + addedIntensity;
  assert(newIntensity >= 0);
  setStatusIntensity(status, static_cast<unsigned>(newIntensity));
}

void Hero::reduceStatus(HeroStatus status)
{
  setStatusIntensity(status, statuses[status] - 1);
}

void Hero::resetStatus(HeroStatus status)
{
  setStatusIntensity(status, 0);
}

bool Hero::hasStatus(HeroStatus status) const
{
  return getStatusIntensity(status) > 0;
}

void Hero::setStatusIntensity(HeroStatus status, unsigned newIntensity)
{
  assert(status != HeroStatus::Exhausted && "Exhausted status is computed on the fly");
  const bool canStack = canHaveMultiple(status) || (status == HeroStatus::Might && hasTrait(HeroTrait::Additives));
  if (newIntensity > 1 && !canStack)
    newIntensity = 1;

  const auto oldIntensity = std::exchange(statuses[status], newIntensity);
  if (newIntensity == oldIntensity)
    return;

  if (status == HeroStatus::Momentum)
  {
    const auto delta =
        static_cast<int>(newIntensity > oldIntensity ? newIntensity - oldIntensity : oldIntensity - newIntensity);
    changeDamageBonusPercent(delta);
  }

  if (newIntensity > 0)
  {
    if (status == HeroStatus::CurseImmune)
      resetStatus(HeroDebuff::Cursed);
    else if (status == HeroStatus::ManaBurnImmune)
      resetStatus(HeroDebuff::ManaBurned);
    else if (status == HeroStatus::PoisonImmune)
      resetStatus(HeroDebuff::Poisoned);
  }
  else if (newIntensity == 0)
    statuses.erase(status);

  if (status == HeroStatus::DodgePermanent || status == HeroStatus::DodgeTemporary)
    rerollDodgeNext();

  if (status == HeroStatus::StoneSkin)
    defence.setStoneSkin(clampedTo<uint8_t>(newIntensity));
}

unsigned Hero::getStatusIntensity(HeroStatus status) const
{
  // Compute Exhausted status when needed instead of updating it all the time
  if (status == HeroStatus::Exhausted)
    return hasTrait(HeroTrait::Damned) && getHitPoints() < getHitPointsMax();

  auto iter = statuses.find(status);
  return iter != statuses.end() ? iter->second : 0;
}

void Hero::addStatus(HeroDebuff debuff, Monsters& allMonsters, int addedIntensity)
{
  if (addedIntensity <= 0)
  {
    if (addedIntensity < 0)
    {
      // For convenience, allow to call addStatus with -1 instead of reduceStatus
      assert(addedIntensity == -1);
      reduceStatus(debuff);
    }
    return;
  }

  // Reject changes if there is a corresponding immunity
  if ((debuff == HeroDebuff::Cursed && hasStatus(HeroStatus::CurseImmune)) ||
      (debuff == HeroDebuff::ManaBurned && hasStatus(HeroStatus::ManaBurnImmune)) ||
      (debuff == HeroDebuff::Poisoned && hasStatus(HeroStatus::PoisonImmune)))
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
    defence.setCorrosion(clampedTo<uint8_t>(intensity));
  else if (debuff == HeroDebuff::Cursed)
    defence.setCursed(true);
}

void Hero::reduceStatus(HeroDebuff debuff)
{
  auto iter = debuffs.find(debuff);
  if (iter == debuffs.end() || iter->second == 0)
    return;

  const auto newIntensity = iter->second - 1;

  if (newIntensity == 0)
  {
    resetStatus(debuff);
    return;
  }

  debuffs[debuff] = newIntensity;

  if (debuff == HeroDebuff::Corroded)
    defence.setCorrosion(clampedTo<uint8_t>(newIntensity));
}

void Hero::resetStatus(HeroDebuff debuff)
{
  if (debuffs.erase(debuff))
  {
    if (debuff == HeroDebuff::Corroded)
      defence.setCorrosion(0);
    else if (debuff == HeroDebuff::Cursed)
      defence.setCursed(false);
  }
}

bool Hero::hasStatus(HeroDebuff debuff) const
{
  return getStatusIntensity(debuff) > 0;
}

unsigned Hero::getStatusIntensity(HeroDebuff debuff) const
{
  auto iter = debuffs.find(debuff);
  return iter != debuffs.end() ? iter->second : 0;
}

void Hero::addTrait(HeroTrait trait)
{
  if (hasTrait(trait))
  {
    assert(false);
    return;
  }
  traits.emplace_back(trait);
}

bool Hero::hasTrait(HeroTrait trait) const
{
  return std::find(begin(traits), end(traits), trait) != end(traits);
}

void Hero::monsterKilled(const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning, Monsters& allMonsters)
{
  assert(monster.isDefeated());
  addStatus(HeroDebuff::Cursed, allMonsters, monster.has(MonsterTrait::CurseBearer) ? 1 : -1);
  applyOrCollect(faith.monsterKilled(monster, getLevel(), monsterWasBurning), allMonsters);
  if (monster.grantsXP())
  {
    gainExperienceForKill(monster.getLevel(), monsterWasSlowed, allMonsters);
    if (has(ShopItem::GlovesOfMidas))
      ++inventory.gold;
    if (has(ShopItem::BlueBead) && !hasStatus(HeroDebuff::ManaBurned))
      recoverManaPoints(1);
  }
  if (has(ShopItem::StoneSigil))
    faith.gainPiety(1);
}

void Hero::adjustMomentum(bool increase)
{
  if (hasTrait(HeroTrait::Momentum))
  {
    if (increase)
      addStatus(HeroStatus::Momentum, 15);
    else
    {
      const auto momentum = static_cast<int>(getStatusIntensity(HeroStatus::Momentum));
      const auto delta = -(momentum + 1) / 2;
      addStatus(HeroStatus::Momentum, delta);
    }
  }
}

void Hero::removeOneTimeAttackEffects()
{
  resetStatus(HeroStatus::ConsecratedStrike);
  resetStatus(HeroStatus::CrushingBlow);
  resetStatus(HeroStatus::Might);
  resetStatus(HeroStatus::SpiritStrength);
  resetStatus(HeroStatus::FirstStrikeTemporary);
  resetStatus(HeroStatus::Reflexes);

  if (inventory.triswordUsed())
    changeBaseDamage(-1);
}

void Hero::levelGainedUpdate(unsigned newLevel, Monsters& allMonsters)
{
  const int newHpMax = static_cast<int>(stats.getHitPointsMax()) + 10 + stats.getHealthBonus();
  assert(newHpMax > 0);
  stats.setHitPointsMax(static_cast<unsigned>(newHpMax));
  const int addedBaseDamage = [&] {
    if (hasTrait(HeroTrait::HandToHand))
      return 3;
    else if (hasTrait(HeroTrait::RegalSize))
      return 1;
    return 5;
  }();
  changeBaseDamage(addedBaseDamage);
  if (hasTrait(HeroTrait::RegalHygiene))
    addStatus(HeroStatus::CorrosiveStrike);
  if (has(ShopItem::Platemail))
    addStatus(HeroStatus::DamageReduction, 2);
  if (has(ShopItem::MartyrWraps))
  {
    addStatus(HeroDebuff::Corroded, allMonsters);
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
  resetStatus(HeroDebuff::Poisoned);
  resetStatus(HeroDebuff::ManaBurned);

  if (has(MiscItem::PatchesTheTeddy) && !hasStatus(HeroStatus::Pessimist))
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
      addStatus(HeroDebuff::Poisoned, allMonsters);
      break;
    case 1:
      addStatus(HeroDebuff::ManaBurned, allMonsters);
      break;
    case 2:
      // not implemented: random teleport
      break;
    case 3:
      // not implemented: grid reveal (rare?)
      break;
    }
  }

  if (!hasTrait(HeroTrait::Prototype))
    stats.refresh();
}

void Hero::addDodgeChancePercent(unsigned percent, bool isPermanent)
{
  const auto statusToUpdate = isPermanent ? HeroStatus::DodgePermanent : HeroStatus::DodgeTemporary;
  const auto newDodgeChance = std::min(getStatusIntensity(statusToUpdate) + percent, 100u);
  setStatusIntensity(statusToUpdate, newDodgeChance);
}

unsigned Hero::getDodgeChancePercent() const
{
  const unsigned dodgeChance =
      std::min(getStatusIntensity(HeroStatus::DodgePermanent) + getStatusIntensity(HeroStatus::DodgeTemporary), 100u);
  if (hasStatus(HeroStatus::Pessimist) && dodgeChance != 100u)
    return 0;
  return dodgeChance;
}

bool Hero::predictDodgeNext() const
{
  assert(hasStatus(HeroStatus::DodgePrediction) && "predictDodgeNext called without dodge prediction status");
  if (hasStatus(HeroStatus::Pessimist))
    return false;
  return dodgeNext;
}

bool Hero::tryDodge(Monsters& allMonsters)
{
  const bool success = dodgeNext && (!hasStatus(HeroStatus::Pessimist) || getDodgeChancePercent() == 100);
  rerollDodgeNext();
  if (success)
  {
    resetStatus(HeroStatus::DodgePrediction);
    resetStatus(HeroStatus::DodgeTemporary);
    applyOrCollect(faith.dodgedAttack(), allMonsters);
  }
  return success;
}

void Hero::wallDestroyed()
{
  if (has(Boon::StoneForm))
    addStatus(HeroStatus::Might);
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
  const auto sanguine = getStatusIntensity(HeroStatus::Sanguine);
  if (sanguine == 0)
    return false;
  healHitPoints(getHitPointsMax() * sanguine / 100, false);
  Monsters ignore;
  applyOrCollect(faith.bloodPoolConsumed(receivedBoonCount(Boon::BloodTithe)), ignore);
  if (hasTrait(HeroTrait::Insane))
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
  if (hasStatus(HeroStatus::Pessimist))
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
  if (hasTrait(HeroTrait::BlackMarket))
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
  if (hasTrait(HeroTrait::RegalPerks))
    healHitPoints(getHitPointsMax() / 2, true);
  receive(item);
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

bool Hero::followDeity(God god, unsigned numRevealedTiles)
{
  return faith.followDeity(god, *this, numRevealedTiles);
}

bool Hero::request(BoonOrPact boonOrPact, Monsters& allMonsters, Resources& resources)
{
  if (const auto boon = std::get_if<Boon>(&boonOrPact))
    return faith.request(*boon, *this, allMonsters, resources);
  return faith.enter(std::get<Pact>(boonOrPact));
}

void Hero::desecrate(God altar, Monsters& allMonsters)
{
  faith.desecrate(altar, *this, allMonsters, has(ShopItem::AgnosticCollar));
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
  stats.setHitPointsMax(hitPointsMax);
}

void Hero::setManaPointsMax(unsigned manaPointsMax)
{
  stats.setManaPointsMax(manaPointsMax);
}

void Hero::changeHitPointsMax(int deltaPoints)
{
  stats.setHitPointsMax(static_cast<unsigned>(static_cast<int>(stats.getHitPointsMax()) + deltaPoints));
}

void Hero::changeManaPointsMax(int deltaPoints)
{
  stats.setManaPointsMax(static_cast<unsigned>(static_cast<int>(stats.getManaPointsMax()) + deltaPoints));
}

void Hero::changePhysicalResistPercentMax(int deltaPoints)
{
  defence.setPhysicalResistPercentMax(clampedTo<uint8_t>(defence.getPhysicalResistPercentMax() + deltaPoints));
}

void Hero::changeMagicalResistPercentMax(int deltaPoints)
{
  defence.setMagicalResistPercentMax(clampedTo<uint8_t>(defence.getMagicalResistPercentMax() + deltaPoints));
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

void Hero::receiveFreeSpell(Spell spell)
{
  inventory.addFree(spell);
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
  resetStatus(HeroDebuff::Cursed);
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

std::vector<std::pair<Item, int>> Hero::getItemCounts() const
{
  return inventory.getItemCounts();
}

std::vector<std::pair<Spell, int>> Hero::getSpellCounts() const
{
  return inventory.getSpellCounts();
}

std::vector<std::pair<Inventory::Entry, int>> Hero::getItemsGrouped() const
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

void Hero::receive(ItemOrSpell itemOrSpell)
{
  inventory.add(itemOrSpell);
  if (const auto item = std::get_if<Item>(&itemOrSpell))
    changeStatsFromItem(*item, true);
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
    return !hasStatus(HeroStatus::DeathProtection);

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
  return item != BossReward::NamtarsWard || (!hasStatus(HeroStatus::DeathProtection) && !namtarsWardUsedThisLevel);
}

void Hero::use(Potion potion, Monsters& allMonsters)
{
  switch (potion)
  {
  case Potion::HealthPotion:
    drinkHealthPotion();
    break;
  case Potion::ManaPotion:
    drinkManaPotion();
    break;
  case Potion::FortitudeTonic:
    resetStatus(HeroDebuff::Poisoned);
    resetStatus(HeroDebuff::Weakened);
    break;
  case Potion::BurnSalve:
    resetStatus(HeroDebuff::ManaBurned);
    resetStatus(HeroDebuff::Corroded);
    break;
  case Potion::StrengthPotion:
    addSpiritStrength();
    break;
  case Potion::Schadenfreude:
    addStatus(HeroStatus::Schadenfreude);
    break;
  case Potion::QuicksilverPotion:
    addStatus(HeroStatus::DodgePrediction);
    addStatus(HeroStatus::DodgeTemporary, 50);
    break;
  case Potion::ReflexPotion:
    addStatus(HeroStatus::Reflexes);
    break;
  case Potion::CanOfWhupaz:
    addStatus(HeroStatus::CrushingBlow);
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
    if (!hasStatus(HeroStatus::DeathProtection))
    {
      addStatus(HeroStatus::DeathProtection);
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
    inventory.add(Potion::HealthPotion);
    inventory.add(Potion::HealthPotion);
    inventory.add(Potion::HealthPotion);
    consumed = true;
    break;
  case ShopItem::KegOfMana:
    inventory.add(Potion::ManaPotion);
    inventory.add(Potion::ManaPotion);
    inventory.add(Potion::ManaPotion);
    consumed = true;
    break;
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
    addStatus(HeroStatus::DeathProtection);
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
    addGold(hasTrait(HeroTrait::BlackMarket) ? 11 : 10);
    return true;
  }
  return false;
}

bool Hero::useTranslocationSealOn(Item shopItem)
{
  if (has(AlchemistSeal::TranslocationSeal))
  {
    if (!isSmall(shopItem) && inventory.numFreeSmallSlots() < 4 && !hasTrait(HeroTrait::RegalSize))
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
    addStatus(HeroStatus::PiercePhysical, sign);
    addStatus(HeroStatus::SlowStrike, sign);
    break;
  case BlacksmithItem::BearMace:
    addStatus(HeroStatus::Knockback, 25 * sign);
    break;
  case BlacksmithItem::Sword:
    changeBaseDamage(2 * sign);
    break;
  case BlacksmithItem::Shield:
    addStatus(HeroStatus::DamageReduction, 2 * sign);
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
    addStatus(HeroStatus::DamageReduction, 2 * sign * static_cast<int>(getLevel()));
    addStatus(HeroStatus::SlowStrike, sign);
    break;
  case ShopItem::Whurrgarbl:
    addStatus(HeroStatus::BurningStrike, sign);
    break;
  case ShopItem::Trisword:
    changeBaseDamage(sign * inventory.getTriswordDamage());
    break;
  case ShopItem::VenomDagger:
    addStatus(HeroStatus::Poisonous, 2 * sign);
    break;
  case ShopItem::MartyrWraps:
    addStatus(HeroStatus::CorrosiveStrike, sign);
    break;
  case ShopItem::MagePlate:
  {
    const int strength = (getLevel() + 1) / 2;
    changeManaPointsMax(sign * strength);
    changeDamageBonusPercent(-5 * sign * strength);
    break;
  }
  case ShopItem::VampiricBlade:
    addStatus(HeroStatus::LifeSteal, sign);
    break;
  case ShopItem::ViperWard:
    if (itemReceived)
      addStatus(HeroStatus::PoisonImmune);
    else if (!hasTrait(HeroTrait::Scars) && !hasTrait(HeroTrait::Undead))
      resetStatus(HeroStatus::PoisonImmune);
    break;
  case ShopItem::SoulOrb:
    if (itemReceived)
      addStatus(HeroStatus::ManaBurnImmune);
    else if (!hasTrait(HeroTrait::Scars) && !hasTrait(HeroTrait::Undead))
      resetStatus(HeroStatus::ManaBurnImmune);
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
      addStatus(HeroStatus::DeathGazeImmune);
    else if (!hasTrait(HeroTrait::AzureBody))
      resetStatus(HeroStatus::DeathGazeImmune);
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
    addStatus(HeroStatus::HeavyFireball, sign);
}

void Hero::changeStatsImpl(TaurogItem item, bool itemReceived)
{
  switch (item)
  {
  case TaurogItem::Skullpicker:
    changeBaseDamage(itemReceived ? +5 : -5);
    break;
  case TaurogItem::Wereward:
    addStatus(HeroStatus::DamageReduction, itemReceived ? +5 : -5);
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

int Hero::getConversionPoints() const
{
  return conversion.getPoints();
}

int Hero::getConversionThreshold() const
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
      std::to_string(hero.getDamageVersusStandard()) + " damage (" + std::to_string(hero.getBaseDamage()) + "+" +
          std::to_string(hero.getDamageBonusPercent()) + "%)",
      std::to_string(hero.getPiety()) + " piety",
      std::to_string(hero.gold()) + " gold"};
  if (hero.getPhysicalResistPercent() > 0)
    description.emplace_back(std::to_string(hero.getPhysicalResistPercent()) + "% physical resist");
  if (hero.getMagicalResistPercent() > 0)
    description.emplace_back(std::to_string(hero.getMagicalResistPercent()) + "% magic resist");

  for (int i = 0; i < static_cast<int>(HeroDebuff::Last); ++i)
  {
    const auto debuff = static_cast<HeroDebuff>(i);
    const auto intensity = hero.getStatusIntensity(debuff);
    if (intensity > 0)
    {
      if (canHaveMultiple(debuff) && intensity > 1)
        description.emplace_back("is "s + toString(debuff) + " (x" + std::to_string(intensity) + ")");
      else
        description.emplace_back("is "s + toString(debuff));
    }
  }

  const bool isPessimist = hero.hasStatus(HeroStatus::Pessimist);
  for (int i = 0; i < static_cast<int>(HeroStatus::Last); ++i)
  {
    const auto status = static_cast<HeroStatus>(i);
    const auto intensity = hero.getStatusIntensity(status);
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
    const auto intensityBefore = before.getStatusIntensity(debuff);
    const auto intensityNow = now.getStatusIntensity(debuff);
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
    const auto intensityBefore = before.getStatusIntensity(status);
    const auto intensityNow = now.getStatusIntensity(status);
    if (intensityBefore == intensityNow)
      continue;
    if ((now.hasStatus(HeroStatus::Pessimist) &&
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
      const bool usePast = !now.hasStatus(status);
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
