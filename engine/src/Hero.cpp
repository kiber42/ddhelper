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
    addStatus(HeroStatus::FirstStrike);
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

void Hero::gainExperience(int xpGained, bool monsterWasSlowed)
{
  int level = experience.getLevel();
  if (xpGained > 0)
  {
    const int bonuses =
        getStatusIntensity(HeroStatus::Learning) + (int)hasTrait(HeroTrait::Veteran) + (int)monsterWasSlowed;
    experience.gain(xpGained, bonuses, hasStatus(HeroStatus::ExperienceBoost));
    removeStatus(HeroStatus::ExperienceBoost, true);
  }
  else if (monsterWasSlowed)
  {
    // Slowed monster killed by petrification
    experience.gain(0, 1, false);
  }
  while (getLevel() > level)
  {
    levelGainedUpdate();
    ++level;
  }
}

void Hero::gainExperienceNoBonuses(int xpGained)
{
  int level = experience.getLevel();
  experience.gain(0, xpGained, false);
  while (getLevel() > level)
  {
    levelGainedUpdate();
    ++level;
  }
}

void Hero::gainLevel()
{
  const int initialLevel = getLevel();
  experience.gainLevel();
  if (getLevel() > initialLevel)
    levelGainedUpdate();
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
  if (hasTrait(HeroTrait::GoodDrink))
    stats.healHitPoints(getHitPointsMax(), false);
  else
    stats.healHitPoints(getHitPointsMax() * 4 / 10, false);
  removeStatus(HeroStatus::Poisoned, true);
  if (hasTrait(HeroTrait::Survivor))
    stats.recoverManaPoints(getManaPointsMax() * 2 / 10);
}

void Hero::drinkManaPotion()
{
  if (hasTrait(HeroTrait::PowerHungry))
  {
    stats.recoverManaPoints(getManaPointsMax() * 6 / 10);
    addStatus(HeroStatus::Sanguine, 2);
  }
  else
    stats.recoverManaPoints(getManaPointsMax() * 4 / 10);
  removeStatus(HeroStatus::ManaBurned, true);
  if (hasTrait(HeroTrait::Courageous))
    addStatus(HeroStatus::Might);
  if (hasTrait(HeroTrait::Survivor))
    stats.healHitPoints(getHitPointsMax() * 2 / 10, false);
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
  if (hasTrait(HeroTrait::Determined) && stats.getHitPoints() < stats.getHitPointsMax() / 2)
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
  setPhysicalResistPercent(getPhysicalResistPercent() + deltaPercent);
}

void Hero::changeMagicalResistPercent(int deltaPercent)
{
  setMagicalResistPercent(getMagicalResistPercent() + deltaPercent);
}

bool Hero::doesMagicalDamage() const
{
  return hasStatus(HeroStatus::ConsecratedStrike) || hasStatus(HeroStatus::MagicalAttack);
}

bool Hero::hasInitiativeVersus(const Monster& monster) const
{
  // TODO Adjust behaviour based on hero's traits
  if (hasStatus(HeroStatus::Reflexes))
    return true;

  const bool heroFast = hasStatus(HeroStatus::FirstStrike) && !hasStatus(HeroStatus::SlowStrike);
  const bool monsterFast = monster.hasFirstStrike() && !monster.isSlowed();
  if (heroFast || monsterFast)
    return !monsterFast;

  const bool heroSlow = !hasStatus(HeroStatus::FirstStrike) && hasStatus(HeroStatus::SlowStrike);
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
  {
    int multiplier = getLevel() * (hasTrait(HeroTrait::Discipline) ? 2 : 1);
    if (hasTrait(HeroTrait::Discipline))
      multiplier *= 2;
    if (hasTrait(HeroTrait::Damned))
      multiplier = 1;
    // TODO: Add +1 for Bloody Sigil
    stats.healHitPoints(nSquares * multiplier, false);
  }
  if (!hasStatus(HeroStatus::ManaBurned) && !exhausted)
  {
    stats.recoverManaPoints(nSquares);
  }
}

int Hero::numSquaresForFullRecovery() const
{
  int numHP = 0;
  if (!hasStatus(HeroStatus::Poisoned))
  {
    int multiplier = getLevel() * (hasTrait(HeroTrait::Discipline) ? 2 : 1);
    if (hasTrait(HeroTrait::Damned))
      multiplier = 1;
    // TODO: Add +1 for Bloody Sigil
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

void Hero::fullHealthAndMana()
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

  statuses[status] = newIntensity;
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
      pietyChange += faith.becameManaBurned();
      const int mp = getManaPoints();
      pietyChange += faith.manaPointsBurned(mp);
      loseManaPoints(mp);
      applyOrCollect(pietyChange);
    }
    else if (status == HeroStatus::Poisoned)
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

bool Hero::isTraitActive(HeroTrait trait) const
{
  if (!hasTrait(trait))
    return false;
  if (trait == HeroTrait::Determined)
    return stats.getHitPoints() * 2 < stats.getHitPointsMax();
  if (trait == HeroTrait::Courageous)
    throw std::runtime_error("Not implemented");
  // Most traits are always active
  return true;
}

void Hero::removeOneTimeAttackEffects()
{
  removeStatus(HeroStatus::ConsecratedStrike, true);
  removeStatus(HeroStatus::CrushingBlow, true);
  removeStatus(HeroStatus::Might, true);
  removeStatus(HeroStatus::SpiritStrength, true);

  if (!hasTrait(HeroTrait::Dexterous))
    removeStatus(HeroStatus::FirstStrike, true);
  removeStatus(HeroStatus::FirstStrikeTemporary, true);
  removeStatus(HeroStatus::Reflexes, true);
}

void Hero::levelGainedUpdate()
{
  stats.setHitPointsMax(stats.getHitPointsMax() + 10 + stats.getHealthBonus());
  stats.refresh();
  removeStatus(HeroStatus::Poisoned, true);
  removeStatus(HeroStatus::ManaBurned, true);
  changeBaseDamage(hasTrait(HeroTrait::HandToHand) ? +3 : +5);
  applyOrCollect(faith.levelGained());
}

void Hero::addDodgeChancePercent(int percent, bool isPermanent)
{
  const int permanent = getStatusIntensity(HeroStatus::DodgePermanent);
  const int temporary = getStatusIntensity(HeroStatus::DodgeTemporary);
  if (isPermanent)
    setStatusIntensity(HeroStatus::DodgePermanent, std::min(std::max(permanent + percent, 0), 100 - temporary));
  else
    setStatusIntensity(HeroStatus::DodgeTemporary, std::min(std::max(temporary + percent, 0), 100 - permanent));
}

int Hero::getDodgeChancePercent() const
{
  return getStatusIntensity(HeroStatus::DodgePermanent) + getStatusIntensity(HeroStatus::DodgeTemporary);
}

bool Hero::predictDodgeNext() const
{
  assert(hasStatus(HeroStatus::DodgePrediction) && "predictDodgeNext called without dodge prediction status");
  return dodgeNext;
}

bool Hero::tryDodge()
{
  const bool success = dodgeNext;
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

int Hero::gold() const
{
  return stats.gold;
}

void Hero::addGold(int amountAdded)
{
  stats.gold += amountAdded;
  if (stats.gold < 0)
    stats.gold = 0;
}

bool Hero::spendGold(int amountSpent)
{
  if (stats.gold < amountSpent)
    return false;
  stats.gold -= amountSpent;
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
  faith.desecrate(altar, *this);
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
    applyOrCollect(faith.converted(item));
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
    applyOrCollect(faith.converted(spell));
  }
}

bool Hero::canUse(Item item) const
{
  switch (item)
  {
  // Basic Items
  case Item::BadgeOfHonour:
    return !hasStatus(HeroStatus::DeathProtection);

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

  default:
    return false;
  }
}

void Hero::use(Item item)
{
  bool consumed = true;

  switch (item)
  {
  // Basic Items
  case Item::BadgeOfHonour:
    if (hasStatus(HeroStatus::DeathProtection))
      consumed = false;
    else
      addStatus(HeroStatus::DeathProtection);

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

  default:
    break;
  }

  applyOrCollect(faith.itemUsed(item));

  if (consumed)
  {
    inventory.remove(item);
    changeStatsFromItem(item, false);
  }
}

void Hero::changeStatsFromItem(Item item, bool itemReceived)
{
  // Apply passive item effects on hero status
  switch (item)
  {
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
