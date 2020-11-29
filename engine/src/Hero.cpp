#include "Hero.hpp"

#include "Experience.hpp"
#include "Items.hpp"
#include "Monster.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

Hero::Hero()
  : Hero(HeroClass::Guard, HeroRace::Human)
{
}

Hero::Hero(HeroClass theClass, HeroRace race)
  : name(isMonsterClass(theClass) ? toString(theClass) : (toString(race) + std::string(" ") + toString(theClass)))
  , stats()
  , defence(0, 0, 65, 65)
  , experience()
  , inventory()
  , conversion(theClass, race)
  , faith()
  , statuses()
  , traits(startingTraits(theClass))
  , collectedPiety()
  , generator(std::random_device{}())
  , dodgeNext(false)
  , alchemistScrollUsedThisLevel(false)
  , namtarsWardUsedThisLevel(false)
{
  if (hasTrait(HeroTrait::Veteran))
    experience = Experience(Experience::IsVeteran{});
  if (hasTrait(HeroTrait::Dangerous))
    stats = HeroStats(HeroStats::IsDangerous{});

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
  if (hasTrait(HeroTrait::Scars))
  {
    addStatus(HeroStatus::PoisonImmune);
    addStatus(HeroStatus::ManaBurnImmune);
    addStatus(HeroStatus::CurseImmune);
  }

  if (hasTrait(HeroTrait::Defiant))
    inventory.add(Spell::Cydstepp);
  if (hasTrait(HeroTrait::MagicAttunement))
    inventory.add(Spell::Burndayraz);
  if (hasTrait(HeroTrait::Insane))
    inventory.add(Spell::Bludtupowa);
  if (hasTrait(HeroTrait::PoisonedBlade))
    inventory.add(Spell::Apheelsik);
  if (hasTrait(HeroTrait::HolyHands))
    inventory.add(Spell::Halpmeh);
}

Hero::Hero(HeroStats stats, Defence defence, Experience experience)
  : name("Hero")
  , stats(std::move(stats))
  , defence(std::move(defence))
  , experience(std::move(experience))
  , inventory()
  , conversion(HeroClass::Guard, HeroRace::Human)
  , faith()
  , statuses()
  , traits()
  , collectedPiety()
  , generator(std::random_device{}())
  , dodgeNext(false)
  , alchemistScrollUsedThisLevel(false)
  , namtarsWardUsedThisLevel(false)
{
}

std::string Hero::getName() const
{
  return name;
}

int Hero::getXP() const
{
  return experience.getXP();
}

int Hero::getLevel() const
{
  return experience.getLevel();
}

int Hero::getPrestige() const
{
  return experience.getPrestige();
}

int Hero::getXPforNextLevel() const
{
  return experience.getXPforNextLevel();
}

void Hero::gainExperienceForKill(int monsterLevel, bool monsterWasSlowed)
{
  const int xpBase = Experience::forHeroAndMonsterLevels(getLevel(), monsterLevel);
  int xpBonuses = getStatusIntensity(HeroStatus::Learning);
  if (monsterWasSlowed)
    ++xpBonuses;
  if (hasTrait(HeroTrait::Veteran))
    ++xpBonuses;
  if (has(Item::BalancedDagger) && getLevel() == monsterLevel)
    xpBonuses += 2;
  gainExperience(xpBase, xpBonuses);
}

void Hero::gainExperienceForPetrification(bool monsterWasSlowed)
{
  if (monsterWasSlowed)
    gainExperience(0, 1);
}

void Hero::gainExperienceNoBonuses(int xpGained)
{
  gainExperience(0, xpGained);
}

void Hero::gainExperience(int xpBase, int xpBonuses)
{
  int level = getLevel();
  const int prestige = getPrestige();
  experience.gain(xpBase, xpBonuses, hasStatus(HeroStatus::ExperienceBoost));
  if (xpBase > 0)
    removeStatus(HeroStatus::ExperienceBoost, true);
  const bool levelUp = getLevel() > level || getPrestige() > prestige;
  while (getLevel() > level)
  {
    ++level;
    levelGainedUpdate(level);
  }
  if (levelUp)
    levelUpRefresh();
}

void Hero::gainLevel()
{
  const int initialLevel = getLevel();
  experience.gainLevel();
  if (getLevel() > initialLevel)
    levelGainedUpdate(getLevel());
  levelUpRefresh();
}

bool Hero::isDefeated() const
{
  return stats.isDefeated();
}

int Hero::getHitPoints() const
{
  return stats.getHitPoints();
}

int Hero::getHitPointsMax() const
{
  return stats.getHitPointsMax();
}

int Hero::getManaPoints() const
{
  return stats.getManaPoints();
}

int Hero::getManaPointsMax() const
{
  return stats.getManaPointsMax();
}

void Hero::drinkHealthPotion()
{
  const bool hasNagaCauldron = has(Item::NagaCauldron);
  int percentHealed = hasTrait(HeroTrait::GoodDrink) ? 100 : 40;
  if (hasNagaCauldron)
    percentHealed += nagaCauldronBonus();
  stats.healHitPoints(getHitPointsMax() * percentHealed / 100, hasNagaCauldron);
  removeStatus(HeroStatus::Poisoned, true);
  if (hasTrait(HeroTrait::Survivor))
    stats.recoverManaPoints(getManaPointsMax() * 2 / 10);
}

void Hero::drinkManaPotion()
{
  const bool hasNagaCauldron = has(Item::NagaCauldron);
  int percentRestored = 40;
  if (hasTrait(HeroTrait::PowerHungry))
  {
    percentRestored = 60;
    addStatus(HeroStatus::Sanguine, 2);
  }
  if (hasNagaCauldron)
    percentRestored += nagaCauldronBonus();
  stats.recoverManaPoints(getManaPointsMax() * percentRestored / 100);
  removeStatus(HeroStatus::ManaBurned, true);
  if (hasTrait(HeroTrait::Courageous))
    addStatus(HeroStatus::Might);
  if (hasTrait(HeroTrait::Survivor))
    stats.healHitPoints(getHitPointsMax() * 2 / 10, false);
}

int Hero::nagaCauldronBonus() const
{
  return 5 * (static_cast<int>(hasStatus(HeroStatus::Poisoned)) + static_cast<int>(hasStatus(HeroStatus::ManaBurned)) +
              static_cast<int>(hasStatus(HeroStatus::Corrosion)) + static_cast<int>(hasStatus(HeroStatus::Weakened)) +
              static_cast<int>(hasStatus(HeroStatus::Cursed)));
}

int Hero::getBaseDamage() const
{
  const int modifiers = getStatusIntensity(HeroStatus::SpiritStrength) - getStatusIntensity(HeroStatus::Weakened);
  return std::max(stats.getBaseDamage() + modifiers, 0);
}

void Hero::changeBaseDamage(int deltaDamagePoints)
{
  stats.setBaseDamage(std::max(stats.getBaseDamage() + deltaDamagePoints, 0));
}

int Hero::getDamageBonusPercent() const
{
  int bonus = stats.getDamageBonusPercent();
  if (hasStatus(HeroStatus::Might))
    bonus += 30;
  if (hasTrait(HeroTrait::Determined) && stats.getHitPoints() * 2 < stats.getHitPointsMax())
    bonus += 30;
  return bonus;
}

void Hero::changeDamageBonusPercent(int deltaDamageBonusPercent)
{
  stats.setDamageBonusPercent(stats.getDamageBonusPercent() + deltaDamageBonusPercent);
}

int Hero::getDamageVersusStandard() const
{
  return getBaseDamage() * (100 + getDamageBonusPercent()) / 100;
}

int Hero::getDamageOutputVersus(const Monster& monster) const
{
  const int standardDamage = getDamageVersusStandard();
  int damage = standardDamage;
  if (hasTrait(HeroTrait::Bloodlust) && monster.getLevel() > getLevel())
    damage += standardDamage * 2 / 10;
  if (hasTrait(HeroTrait::Stabber) && monster.getHitPoints() >= monster.getHitPointsMax())
    damage += standardDamage * 3 / 10;
  if (hasTrait(HeroTrait::PoisonedBlade) && monster.isPoisoned())
    damage += standardDamage * 4 / 10;
  if (hasTrait(HeroTrait::GoodGolly) && monster.isUndead())
    damage += getBaseDamage();
  return damage;
}

void Hero::addHealthBonus()
{
  stats.addHealthBonus(experience.getUnmodifiedLevel());
}

void Hero::addManaBonus()
{
  stats.setManaPointsMax(stats.getManaPointsMax() + 1);
}

void Hero::addDamageBonus()
{
  changeDamageBonusPercent(+10);
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
  defence.setPhysicalResistPercent(physicalResistPercent);
}

void Hero::setMagicalResistPercent(int magicalResistPercent)
{
  defence.setMagicalResistPercent(magicalResistPercent);
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

bool Hero::hasInitiativeVersus(const Monster& monster) const
{
  if (hasStatus(HeroStatus::Reflexes))
    return true;

  const bool firstStrike = hasStatus(HeroStatus::FirstStrikePermanent) || hasStatus(HeroStatus::FirstStrikeTemporary);
  const bool heroFast = firstStrike && !hasStatus(HeroStatus::SlowStrike);
  const bool monsterFast = monster.hasFirstStrike() && !monster.isSlowed();
  if (heroFast || monsterFast)
    return !monsterFast;

  const bool heroSlow = !firstStrike && hasStatus(HeroStatus::SlowStrike);
  const bool monsterSlow = monster.isSlowed();
  if (heroSlow || monsterSlow)
    return !heroSlow;

  return getLevel() > monster.getLevel();
}

int Hero::predictDamageTaken(int attackerDamageOutput, bool isMagicalDamage) const
{
  return defence.predictDamageTaken(std::max(0, attackerDamageOutput - getStatusIntensity(HeroStatus::DamageReduction)),
                                    isMagicalDamage, 0);
}

void Hero::loseHitPoints(int amountPointsLost)
{
  stats.loseHitPointsWithoutDeathProtection(amountPointsLost);
  if (stats.getHitPoints() == 0 && hasStatus(HeroStatus::DeathProtection))
  {
    stats.barelySurvive();
    removeStatus(HeroStatus::DeathProtection, false);
    applyOrCollect(faith.deathProtectionTriggered());
  }
}

void Hero::takeDamage(int attackerDamageOutput, bool isMagicalDamage)
{
  const int damagePoints = predictDamageTaken(attackerDamageOutput, isMagicalDamage);
  loseHitPoints(damagePoints);
  if (damagePoints > 0 && hasStatus(HeroStatus::Schadenfreude))
  {
    recoverManaPoints(damagePoints);
    removeStatus(HeroStatus::Schadenfreude, true);
  }
  removeStatus(HeroStatus::StoneSkin, true);
}

void Hero::recover(int nSquares)
{
  const bool exhausted = hasStatus(HeroStatus::Exhausted);
  if (!hasStatus(HeroStatus::Poisoned))
    stats.healHitPoints(nSquares * recoveryMultiplier(), false);
  if (!hasStatus(HeroStatus::ManaBurned) && !exhausted)
    stats.recoverManaPoints(nSquares);
}

int Hero::recoveryMultiplier() const
{
  int multiplier = getLevel();
  if (hasTrait(HeroTrait::Discipline))
    multiplier *= 2;
  if (has(Item::BloodySigil))
    multiplier += 1;
  if (hasTrait(HeroTrait::Damned))
    multiplier = 1;
  return multiplier;
}

int Hero::numSquaresForFullRecovery() const
{
  int numHP = 0;
  if (!hasStatus(HeroStatus::Poisoned))
  {
    const int multiplier = recoveryMultiplier();
    numHP = (getHitPointsMax() - getHitPoints() + (multiplier - 1) /* always round up */) / multiplier;
  }
  const int numMP = hasStatus(HeroStatus::ManaBurned) ? 0 : getManaPointsMax() - getManaPoints();
  if (hasTrait(HeroTrait::Damned))
    return numHP + numMP;
  return std::max(numHP, numMP);
}

void Hero::healHitPoints(int amountPointsHealed, bool mayOverheal)
{
  stats.healHitPoints(amountPointsHealed, mayOverheal);
}

void Hero::loseHitPointsOutsideOfFight(int amountPointsLost)
{
  loseHitPoints(amountPointsLost);
}

void Hero::recoverManaPoints(int amountPointsRecovered)
{
  stats.recoverManaPoints(amountPointsRecovered);
}

void Hero::loseManaPoints(int amountPointsLost)
{
  stats.loseManaPoints(amountPointsLost);
}

void Hero::refillHealthAndMana()
{
  stats.refresh();
}

void Hero::addStatus(HeroStatus status, int addedIntensity)
{
  setStatusIntensity(status, statuses[status] + addedIntensity);
}

void Hero::removeStatus(HeroStatus status, bool completely)
{
  if (statuses[status] > 0)
    setStatusIntensity(status, completely ? 0 : statuses[status] - 1);
}

bool Hero::hasStatus(HeroStatus status) const
{
  return getStatusIntensity(status) > 0;
}

void Hero::setStatusIntensity(HeroStatus status, int newIntensity)
{
  assert(status != HeroStatus::Exhausted && "Exhausted status is computed on the fly");
  const bool canStack = canHaveMultiple(status) || (status == HeroStatus::Might && hasTrait(HeroTrait::Additives));
  if (newIntensity > 1 && !canStack)
    newIntensity = 1;
  else if (newIntensity < 0)
    newIntensity = 0;

  // Reject changes if there is a corresponding immunity
  if (newIntensity > 0 && ((status == HeroStatus::Cursed && hasStatus(HeroStatus::CurseImmune)) ||
                           (status == HeroStatus::ManaBurned && hasStatus(HeroStatus::ManaBurnImmune)) ||
                           (status == HeroStatus::Poisoned && hasStatus(HeroStatus::PoisonImmune))))
    return;

  const int oldIntensity = std::exchange(statuses[status], newIntensity);

  if (status == HeroStatus::Momentum)
    changeDamageBonusPercent(newIntensity - oldIntensity);

  if (newIntensity > 0)
  {
    if (status == HeroStatus::CurseImmune)
      removeStatus(HeroStatus::Cursed, true);
    else if (status == HeroStatus::ManaBurnImmune)
      removeStatus(HeroStatus::ManaBurned, true);
    else if (status == HeroStatus::PoisonImmune)
      removeStatus(HeroStatus::Poisoned, true);

    else if (status == HeroStatus::ManaBurned)
    {
      PietyChange pietyChange;
      if (oldIntensity == 0)
        pietyChange += faith.becameManaBurned();
      const int mp = getManaPoints();
      pietyChange += faith.manaPointsBurned(mp);
      loseManaPoints(mp);
      applyOrCollect(pietyChange);
    }
    else if (status == HeroStatus::Poisoned && oldIntensity == 0)
      applyOrCollect(faith.becamePoisoned());
  }
  else if (newIntensity == 0)
    statuses.erase(status);

  if (status == HeroStatus::DodgePermanent || status == HeroStatus::DodgeTemporary)
    rerollDodgeNext();

  propagateStatus(status, newIntensity);
}

int Hero::getStatusIntensity(HeroStatus status) const
{
  // Compute Exhausted status when needed instead of updating it all the time
  if (status == HeroStatus::Exhausted)
    return hasTrait(HeroTrait::Damned) && getHitPoints() < getHitPointsMax();

  auto iter = statuses.find(status);
  return iter != statuses.end() ? iter->second : 0;
}

void Hero::propagateStatus(HeroStatus status, int intensity)
{
  switch (status)
  {
  case HeroStatus::Corrosion:
    defence.setCorrosion(intensity);
    break;
  case HeroStatus::Cursed:
    defence.setCursed(intensity > 0);
    break;
  case HeroStatus::StoneSkin:
    defence.setStoneSkin(intensity);
  default:
    break;
  }
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

void Hero::monsterKilled(const Monster& monster, bool monsterWasSlowed, bool monsterWasBurning)
{
  gainExperienceForKill(monster.getLevel(), monsterWasSlowed);
  applyOrCollect(faith.monsterKilled(monster, getLevel(), monsterWasBurning));
  if (has(Item::GlovesOfMidas))
    ++inventory.gold;
  if (has(Item::StoneSigil))
    faith.gainPiety(1);
  if (has(Item::BlueBead))
    recoverManaPoints(1);
}

void Hero::adjustMomentum(bool increase)
{
  if (hasTrait(HeroTrait::Momentum))
  {
    if (increase)
      addStatus(HeroStatus::Momentum, 15);
    else
    {
      const int momentum = getStatusIntensity(HeroStatus::Momentum);
      const int delta = -(momentum + 1) / 2;
      addStatus(HeroStatus::Momentum, delta);
    }
  }
}

void Hero::removeOneTimeAttackEffects()
{
  removeStatus(HeroStatus::ConsecratedStrike, true);
  removeStatus(HeroStatus::CrushingBlow, true);
  removeStatus(HeroStatus::Might, true);
  removeStatus(HeroStatus::SpiritStrength, true);
  removeStatus(HeroStatus::FirstStrikeTemporary, true);
  removeStatus(HeroStatus::Reflexes, true);

  if (inventory.triswordUsed())
    changeBaseDamage(-1);
}

void Hero::levelGainedUpdate(int newLevel)
{
  stats.setHitPointsMax(stats.getHitPointsMax() + 10 + stats.getHealthBonus());
  changeBaseDamage(hasTrait(HeroTrait::HandToHand) ? +3 : +5);
  if (has(Item::Platemail))
    addStatus(HeroStatus::DamageReduction, 2);
  if (has(Item::MartyrWraps))
  {
    addStatus(HeroStatus::Corrosion);
    // TODO: Corrode all visible monsters
  }
  if (has(Item::MagePlate) && newLevel % 2 == 1)
  {
    changeManaPointsMax(1);
    changeDamageBonusPercent(-5);
  }
  alchemistScrollUsedThisLevel = false;
  namtarsWardUsedThisLevel = false;
  applyOrCollect(faith.levelGained());
  adjustMomentum(false);
}

void Hero::levelUpRefresh()
{
  // TODO: Don't do anything for Goatperson
  removeStatus(HeroStatus::Poisoned, true);
  removeStatus(HeroStatus::ManaBurned, true);
  stats.refresh();
}

void Hero::addDodgeChancePercent(int percent, bool isPermanent)
{
  const auto statusToUpdate = isPermanent ? HeroStatus::DodgePermanent : HeroStatus::DodgeTemporary;
  const int newDodgeChance = std::min(std::max(getStatusIntensity(statusToUpdate) + percent, 0), 100);
  setStatusIntensity(statusToUpdate, newDodgeChance);
}

int Hero::getDodgeChancePercent() const
{
  const int dodgeChance = std::min(getStatusIntensity(HeroStatus::DodgePermanent) + getStatusIntensity(HeroStatus::DodgeTemporary), 100);
  if (hasStatus(HeroStatus::Pessimist) && dodgeChance != 100)
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

bool Hero::tryDodge()
{
  const bool success = dodgeNext && (!hasStatus(HeroStatus::Pessimist) || getDodgeChancePercent() == 100);
  rerollDodgeNext();
  if (success)
  {
    removeStatus(HeroStatus::DodgePrediction, true);
    removeStatus(HeroStatus::DodgeTemporary, true);
    applyOrCollect(faith.dodgedAttack());
  }
  return success;
}

void Hero::rerollDodgeNext()
{
  std::uniform_int_distribution<> number(1, 100);
  dodgeNext = getDodgeChancePercent() >= number(generator);
}

void Hero::applyDragonSoul(int manaCosts)
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

int Hero::gold() const
{
  return inventory.gold;
}

void Hero::addGold(int amountAdded)
{
  inventory.gold += amountAdded;
  if (inventory.gold < 0)
    inventory.gold = 0;
}

bool Hero::spendGold(int amountSpent)
{
  if (inventory.gold < amountSpent)
    return false;
  inventory.gold -= amountSpent;
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

int Hero::getPiety() const
{
  return faith.getPiety();
}

std::optional<God> Hero::getFollowedDeity() const
{
  return faith.getFollowedDeity();
}

bool Hero::hasBoon(Boon boon) const
{
  assert(!allowRepeatedUse(boon));
  return faith.boonCount(boon) > 0;
}

int Hero::receivedBoonCount(Boon boon) const
{
  return faith.boonCount(boon);
}

int Hero::getBoonCosts(Boon boon) const
{
  return faith.getCosts(boon, *this);
}

bool Hero::followDeity(God god)
{
  return faith.followDeity(god, *this);
}

void Hero::desecrate(God altar)
{
  faith.desecrate(altar, *this, has(Item::AgnosticCollar));
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

void Hero::applyCollectedPiety()
{
  assert(collectedPiety);
  faith.apply(*collectedPiety, *this);
  collectedPiety.reset();
}

void Hero::applyOrCollect(PietyChange pietyChange)
{
  if (collectedPiety)
    *collectedPiety += pietyChange;
  else
    faith.apply(pietyChange, *this);
}

void Hero::setHitPointsMax(int hitPointsMax)
{
  stats.setHitPointsMax(hitPointsMax);
}

void Hero::setManaPointsMax(int manaPointsMax)
{
  stats.setManaPointsMax(manaPointsMax);
}

void Hero::changeHitPointsMax(int deltaPoints)
{
  stats.setHitPointsMax(stats.getHitPointsMax() + deltaPoints);
}

void Hero::changeManaPointsMax(int deltaPoints)
{
  stats.setManaPointsMax(stats.getManaPointsMax() + deltaPoints);
}

void Hero::changePhysicalResistPercentMax(int deltaPoints)
{
  defence.setPhysicalResistPercentMax(defence.getPhysicalResistPercentMax() + deltaPoints);
}

void Hero::changeMagicalResistPercentMax(int deltaPoints)
{
  defence.setMagicalResistPercentMax(defence.getMagicalResistPercentMax() + deltaPoints);
}

void Hero::modifyLevelBy(int delta)
{
  experience.modifyLevelBy(delta);
}

void Hero::addConversionPoints(int points)
{
  if (conversion.addPoints(points))
    conversion.applyBonus(*this);
}

bool Hero::lose(Item item)
{
  const bool itemLost = inventory.remove(item) != std::nullopt;
  if (itemLost)
    changeStatsFromItem(item, false);
  return itemLost;
}

void Hero::receiveFreeSpell(Spell spell)
{
  inventory.addFree(spell);
}

void Hero::receiveEnlightenment()
{
  const int enchantedBeads = inventory.enchantPrayerBeads();
  changeHitPointsMax(enchantedBeads);
  changeDamageBonusPercent(+enchantedBeads);
  changeManaPointsMax(+5);
  if (conversion.addPoints(10 * enchantedBeads))
    conversion.applyBonus(*this);
  gainExperienceNoBonuses(enchantedBeads);
  removeStatus(HeroStatus::Cursed, true);
}

void Hero::loseAllItems()
{
  for (auto& entry : getItems())
  {
    for (int i = 0; i < entry.count; ++i)
      changeStatsFromItem(std::get<Item>(entry.itemOrSpell), false);
  }
  inventory.clear();
}

std::vector<Inventory::Entry> Hero::getItems() const
{
  return inventory.getItems();
}

std::vector<Inventory::Entry> Hero::getSpells() const
{
  return inventory.getSpells();
}

bool Hero::has(Item item) const
{
  return inventory.has(item);
}

bool Hero::has(Spell spell) const
{
  return inventory.has(spell);
}

void Hero::receive(Item item)
{
  inventory.add(item);
  changeStatsFromItem(item, true);
}

void Hero::receive(Spell spell)
{
  inventory.add(spell);
}

void Hero::convert(Item item)
{
  const auto conversionValue = inventory.remove(item);
  if (conversionValue.has_value())
  {
    changeStatsFromItem(item, false);
    if (conversion.addPoints(*conversionValue))
      conversion.applyBonus(*this);
    // TODO: Improve inventory system to support compressed items
    applyOrCollect(faith.converted(item, isSmall(item) && !hasTrait(HeroTrait::RegalSize)));
    if (item == Item::Skullpicker || item == Item::Wereward || item == Item::Gloat || item == Item::Will)
      faith.convertedTaurogItem(*this);
  }
}

void Hero::convert(Spell spell)
{
  const auto conversionValue = inventory.remove(spell);
  if (conversionValue.has_value())
  {
    if (hasBoon(Boon::Refreshment))
      recoverManaPoints(getManaPointsMax() * (faith.getFollowedDeity() == God::MysteraAnnur ? 50 : 25) / 100);
    if (conversion.addPoints(*conversionValue))
      conversion.applyBonus(*this);
    // TODO: Improve inventory system to support compressed spells
    applyOrCollect(faith.converted(spell, hasTrait(HeroTrait::MagicSense) && !hasTrait(HeroTrait::RegalSize)));
  }
}

bool Hero::canUse(Item item) const
{
  switch (item)
  {
  // Basic Items
  case Item::BadgeOfHonour:
    return !hasStatus(HeroStatus::DeathProtection);

  // Quest Items
  case Item::FireHeart:
    return inventory.getFireHeartCharge() > 0;
  case Item::CrystalBall:
    return inventory.getCrystalBallCharge() > 0 && gold() >= inventory.getCrystalBallUseCosts();

  // Elite Items
  case Item::KegOfHealth:
  case Item::KegOfMana:
  case Item::AmuletOfYendor:
    return true;

  // TODO: Orb of Zot
  // TODO: Wicked Guitar

  // Potions
  case Item::HealthPotion:
  case Item::ManaPotion:
  case Item::FortitudeTonic:
  case Item::BurnSalve:
  case Item::StrengthPotion:
  case Item::Schadenfreude:
  case Item::QuicksilverPotion:
  case Item::ReflexPotion:
  case Item::CanOfWhupaz:
    return true;

  // Boss Rewards
  case Item::NamtarsWard:
    return !hasStatus(HeroStatus::DeathProtection) && !namtarsWardUsedThisLevel;

  default:
    return false;
  }
}

void Hero::use(Item item)
{
  bool consumed = isPotion(item);

  switch (item)
  {
  // Basic Items
  case Item::BadgeOfHonour:
    if (!hasStatus(HeroStatus::DeathProtection))
    {
      addStatus(HeroStatus::DeathProtection);
      consumed = true;
    }
    break;

  // Quest Items
  case Item::FireHeart:
    healHitPoints(getHitPointsMax() * inventory.fireHeartUsed() / 100);
    break;
  case Item::CrystalBall:
    if (spendGold(inventory.getCrystalBallUseCosts()))
      recoverManaPoints(inventory.crystalBallUsed());
    break;

  // Elite Items
  case Item::KegOfHealth:
    inventory.add(Item::HealthPotion);
    inventory.add(Item::HealthPotion);
    inventory.add(Item::HealthPotion);
    consumed = true;
    break;
  case Item::KegOfMana:
    inventory.add(Item::ManaPotion);
    inventory.add(Item::ManaPotion);
    inventory.add(Item::ManaPotion);
    consumed = true;
    break;
  case Item::AmuletOfYendor:
    gainExperienceNoBonuses(50);
    consumed = true;
    break;

  // TODO: Orb of Zot
  // TODO: Wicked Guitar

  // Potions
  case Item::HealthPotion:
    drinkHealthPotion();
    break;
  case Item::ManaPotion:
    drinkManaPotion();
    break;
  case Item::FortitudeTonic:
    removeStatus(HeroStatus::Poisoned, true);
    removeStatus(HeroStatus::Weakened, true);
    break;
  case Item::BurnSalve:
    removeStatus(HeroStatus::ManaBurned, true);
    removeStatus(HeroStatus::Corrosion, true);
    break;
  case Item::StrengthPotion:
  {
    const int mp = getManaPoints();
    stats.loseManaPoints(mp); // lose MP without other side-effects
    addStatus(HeroStatus::SpiritStrength, getLevel() + mp);
  }
  break;
  case Item::Schadenfreude:
    addStatus(HeroStatus::Schadenfreude);
    break;
  case Item::QuicksilverPotion:
    addStatus(HeroStatus::DodgePrediction);
    addStatus(HeroStatus::DodgeTemporary, 50);
    break;
  case Item::ReflexPotion:
    addStatus(HeroStatus::Reflexes);
    break;
  case Item::CanOfWhupaz:
    addStatus(HeroStatus::CrushingBlow);
    break;

  // Boss Rewards
  case Item::NamtarsWard:
    addStatus(HeroStatus::DeathProtection);
    namtarsWardUsedThisLevel = true;
    break;

  default:
    break;
  }

  applyOrCollect(faith.itemUsed(item));

  if (isPotion(item))
  {
    if (has(Item::Trisword))
      changeBaseDamage(inventory.chargeTrisword());
    if (has(Item::AlchemistScroll) && !alchemistScrollUsedThisLevel && spendGold(3))
    {
      changeHitPointsMax(8);
      alchemistScrollUsedThisLevel = true;
    }
  }

  if (consumed)
  {
    inventory.remove(item);
    changeStatsFromItem(item, false);
  }
}

bool Hero::canUse(Item item, const Monster& monster) const
{
  if (item == Item::SlayerWand)
    return !monster.isDefeated() && monster.getLevel() < 10;
  return canUse(item);
}

void Hero::use(Item item, Monster& monster)
{
  if (item == Item::SlayerWand)
  {
    gainExperienceForKill(std::min(getLevel(), monster.getLevel()), monster.isSlowed());
    monster.die();
    inventory.remove(item);
  }
}

void Hero::changeStatsFromItem(Item item, bool itemReceived)
{
  // Apply passive item effects on hero status
  const int sign = itemReceived ? +1 : -1;
  switch (item)
  {
  case Item::BadgeOfHonour:
    changeDamageBonusPercent(10 * sign);
    break;
  case Item::BloodySigil:
    changeHitPointsMax(5 * sign);
    changeDamageBonusPercent(-10 * sign);
    break;
  case Item::FineSword:
    changeBaseDamage(4 * sign);
    break;
  case Item::PendantOfHealth:
    changeHitPointsMax(10 * sign);
    break;
  case Item::PendantOfMana:
    changeManaPointsMax(2 * sign);
    break;
  case Item::Spoon:
    changeBaseDamage(sign);
    break;
  case Item::TowerShield:
    changePhysicalResistPercent(10 * sign);
    break;
  case Item::TrollHeart:
    // Add 2 additional max HP on future level ups.
    // Was not removed when coverting item in early versions.
    if (itemReceived)
    {
      stats.addHealthBonus(0);
      stats.addHealthBonus(0);
    }
    else
    {
      stats.reduceHealthBonus();
      stats.reduceHealthBonus();
    }
    break;
  case Item::RockHeart:
    // TODO: Recover 1 HP and 1 MP when a wall is destroyed by knockback
    break;
  case Item::HerosHelm:
    changeHitPointsMax(5 * sign);
    changeManaPointsMax(sign);
    changeBaseDamage(2 * sign);
    break;
  case Item::Platemail:
    addStatus(HeroStatus::DamageReduction, 2 * sign * getLevel());
    addStatus(HeroStatus::SlowStrike, sign);
    break;
  case Item::Whurrgarbl:
    addStatus(HeroStatus::BurningStrike, sign);
    break;
  case Item::Trisword:
    changeBaseDamage(sign * inventory.getTriswordDamage());
    break;
  case Item::VenomDagger:
    addStatus(HeroStatus::Poisonous, 2 * sign);
    break;
  case Item::MartyrWraps:
    addStatus(HeroStatus::CorrosiveStrike, sign);
    break;
  case Item::MagePlate:
  {
    const int strength = (getLevel() + 1) / 2;
    changeManaPointsMax(sign * strength);
    changeDamageBonusPercent(-5 * sign * strength);
    break;
  }
  case Item::VampiricBlade:
    addStatus(HeroStatus::LifeSteal, sign);
    break;
  case Item::ViperWard:
    if (itemReceived)
      addStatus(HeroStatus::PoisonImmune);
    else if (!hasTrait(HeroTrait::Scars) && !hasTrait(HeroTrait::Undead))
      removeStatus(HeroStatus::PoisonImmune, true);
    break;
  case Item::SoulOrb:
    if (itemReceived)
      addStatus(HeroStatus::ManaBurnImmune);
    else if (!hasTrait(HeroTrait::Scars) && !hasTrait(HeroTrait::Undead))
      removeStatus(HeroStatus::ManaBurnImmune, true);
    break;
  case Item::ElvenBoots:
    changeManaPointsMax(3 * sign);
    changeMagicalResistPercent(15 * sign);
    break;
  case Item::DwarvenGauntlets:
    changeDamageBonusPercent(20 * sign);
    if (itemReceived)
    {
      stats.addHealthBonus(0);
      stats.addHealthBonus(0);
    }
    else
    {
      stats.reduceHealthBonus();
      stats.reduceHealthBonus();
    }
    break;
  case Item::OrbOfZot:
    changeHitPointsMax(5 * sign);
    changeManaPointsMax(3 * sign);
    break;
  case Item::BearMace:
    addStatus(HeroStatus::Knockback, 25 * sign);
    break;
  case Item::PerseveranceBadge:
    changeDamageBonusPercent(10 * sign);
    break;
  case Item::ReallyBigSword:
    addStatus(HeroStatus::PiercePhysical, sign);
    addStatus(HeroStatus::SlowStrike, sign);
    break;
  case Item::Shield:
    addStatus(HeroStatus::DamageReduction, 2 * sign);
    break;
  case Item::Sword:
    changeBaseDamage(2 * sign);
    break;
  case Item::Gorgward:
    if (itemReceived)
      addStatus(HeroStatus::DeathGazeImmune);
    else if (!hasTrait(HeroTrait::AzureBody))
      removeStatus(HeroStatus::DeathGazeImmune, true);
    break;
  case Item::DragonShield:
    changePhysicalResistPercent(18 * sign);
    changeMagicalResistPercent(18 * sign);
    break;
  case Item::AvatarsCodex:
    addStatus(HeroStatus::HeavyFireball, sign);
    break;
  case Item::Skullpicker:
    changeBaseDamage(itemReceived ? +5 : -5);
    break;
  case Item::Wereward:
    addStatus(HeroStatus::DamageReduction, itemReceived ? +5 : -5);
    break;
  case Item::Gloat:
    changeMagicalResistPercent(itemReceived ? +15 : -15);
    break;
  case Item::Will:
    changePhysicalResistPercent(itemReceived ? +15 : -15);
    break;

  default:
    break;
  }
}

int Hero::getConversionPoints() const
{
  return conversion.getPoints();
}

int Hero::getConversionThreshold() const
{
  return conversion.getThreshold();
}

void Hero::preventDodgeCheat()
{
  if (dodgeNext && !hasStatus(HeroStatus::DodgePrediction))
    rerollDodgeNext();
}
