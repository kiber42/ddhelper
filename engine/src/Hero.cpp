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
  : name(toString(race) + std::string(" ") + toString(theClass))
  , stats()
  , defence(0, 0, 65, 65)
  , experience()
  , inventory()
  , conversion(theClass, race)
  , faith()
  , statuses()
  , traits(startingTraits(theClass))
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
    addDodgeChangePercent(20, true);
  if (hasTrait(HeroTrait::PoisonedBlade))
    changeDamageBonusPercent(-20);

  if (hasTrait(HeroTrait::GoodHealth))
  {
    stats.addHealthBonus(1);
    stats.addHealthBonus(1);
    stats.addHealthBonus(1);
  }

  if (hasTrait(HeroTrait::Defiant))
    inventory.add(Spell::Cydstepp);
  if (hasTrait(HeroTrait::MagicAttunement))
    inventory.add(Spell::Burndayraz);
  if (hasTrait(HeroTrait::Insane))
    inventory.add(Spell::Bludtupowa);
  if (hasTrait(HeroTrait::PoisonedBlade))
    inventory.add(Spell::Apheelsik);
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
  const int bonuses =
      getStatusIntensity(HeroStatus::Learning) + (int)hasTrait(HeroTrait::Veteran) + (int)monsterWasSlowed;
  experience.gain(xpGained, bonuses, hasStatus(HeroStatus::ExperienceBoost));
  removeStatus(HeroStatus::ExperienceBoost, true);
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

int Hero::getDamageVersus(const Monster& monster) const
{
  int damage = getDamageVersusStandard();
  if (hasTrait(HeroTrait::Bloodlust) && monster.getLevel() > getLevel())
    damage = damage * 12 / 10;
  if (hasTrait(HeroTrait::Stabber) && monster.getHitPoints() >= monster.getHitPointsMax())
    damage = damage * 13 / 10;
  if (hasTrait(HeroTrait::PoisonedBlade) && monster.isPoisoned())
    damage = damage * 14 / 10;
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
  return std::min(defence.getPhysicalResistPercent() + 20 * getStatusIntensity(HeroStatus::StoneSkin),
                  defence.getPhysicalResistPercentMax());
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
  if (!hasStatus(HeroStatus::Poisoned))
  {
    int multiplier = getLevel() * (hasTrait(HeroTrait::Discipline) ? 2 : 1);
    if (hasTrait(HeroTrait::Damned))
      multiplier = 1;
    // TODO: Add +1 for Bloody Sigil
    stats.healHitPoints(nSquares * multiplier, false);
  }
  if (!hasStatus(HeroStatus::ManaBurned))
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
    numHP = (getHitPointsMax() - getHitPoints() + multiplier - 1 /* always round up */) / multiplier;
  }
  const int numMP = hasStatus(HeroStatus::ManaBurned) ? 0 : getManaPointsMax() - getManaPoints();
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
  const bool canStack = canHaveMultiple(status) || (status == HeroStatus::Might && hasTrait(HeroTrait::Additives));
  if (newIntensity > 1 && !canStack)
    newIntensity = 1;
  else if (newIntensity < 0)
    newIntensity = 0;
  if (newIntensity == getStatusIntensity(status))
    return;
  if (newIntensity > 0)
  {
    switch (status)
    {
    case HeroStatus::Cursed:
      if (hasStatus(HeroStatus::CurseImmune))
        return;
      break;
    case HeroStatus::CurseImmune:
      removeStatus(HeroStatus::Cursed, true);
      break;
    case HeroStatus::ManaBurned:
      if (hasStatus(HeroStatus::ManaBurnImmune))
        return;
      loseManaPoints(getManaPoints());
      break;
    case HeroStatus::ManaBurnImmune:
      removeStatus(HeroStatus::ManaBurned, true);
      break;
    case HeroStatus::Poisoned:
      if (hasStatus(HeroStatus::PoisonImmune))
        return;
      break;
    case HeroStatus::PoisonImmune:
      removeStatus(HeroStatus::Poisoned, true);
      break;
    default:
      break;
    }
    statuses[status] = newIntensity;
  }
  else
    statuses.erase(status);
  propagateStatus(status, newIntensity);
}

int Hero::getStatusIntensity(HeroStatus status) const
{
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

std::optional<God> Hero::getFollowedDeity() const
{
  return faith.getFollowedDeity();
}

bool Hero::hasBoon(Boon boon) const
{
  return faith.hasBoon(boon);
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
  changeBaseDamage(+5);
}

void Hero::apply(PietyChange pietyChange)
{
  faith.apply(pietyChange, *this);
}

void Hero::addDodgeChangePercent(int percent, bool isPermanent)
{
  const int permanent = getStatusIntensity(HeroStatus::DodgePermanent);
  const int temporary = getStatusIntensity(HeroStatus::DodgeTemporary);
  if (isPermanent)
    setStatusIntensity(HeroStatus::DodgePermanent, std::min(std::max(permanent + percent, 0), 100 - temporary));
  else
    setStatusIntensity(HeroStatus::DodgeTemporary, std::min(std::max(temporary + percent, 0), 100 - permanent));
}

int Hero::getDodgeChangePercent() const
{
  return getStatusIntensity(HeroStatus::DodgePermanent) + getStatusIntensity(HeroStatus::DodgeTemporary);
}

void Hero::modifyLevelBy(int delta)
{
  experience.modifyLevelBy(delta);
}

std::vector<Inventory::Entry> Hero::getItems() const
{
  return inventory.getItems();
}

std::vector<Inventory::Entry> Hero::getSpells() const
{
  return inventory.getSpells();
}

void Hero::setHitPointsMax(int hitPointsMax)
{
  stats.setHitPointsMax(hitPointsMax);
}

void Hero::setManaPointsMax(int manaPointsMax)
{
  stats.setManaPointsMax(manaPointsMax);
}

void Hero::receive(Item item)
{
  inventory.add(item);
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
    if (conversion.addPoints(*conversionValue))
      conversion.applyBonus(*this);
    apply(faith.converted(item));
  }
}

void Hero::convert(Spell spell)
{
  const auto conversionValue = inventory.remove(spell);
  if (conversionValue.has_value())
  {
    if (conversion.addPoints(*conversionValue))
      conversion.applyBonus(*this);
    apply(faith.converted(spell));
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

  if (consumed)
    inventory.remove(item);
}

void Hero::receiveFreeSpell(Spell spell)
{
  inventory.addFree(spell);
}

void Hero::loseAllItems()
{
  inventory.clear();
}
