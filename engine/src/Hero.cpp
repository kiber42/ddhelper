#include "Hero.hpp"

#include "Experience.hpp"
#include "Items.hpp"
#include "Magic.hpp"
#include "Monster.hpp"

#include <algorithm>
#include <cassert>
#include <utility>

Hero::Hero(HeroClass theClass, HeroRace race)
  : name(isMonsterClass(theClass) ? toString(theClass) : (toString(race) + std::string(" ") + toString(theClass)))
  , traits(startingTraits(theClass))
  , stats()
  , defence(0, 0, 65, 65)
  , experience()
  , inventory(6, 100, hasTrait(HeroTrait::MagicSense), hasTrait(HeroTrait::RegalSize), hasTrait(HeroTrait::Negotiator))
  , conversion(theClass, race)
  , faith()
  , statuses()
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
  if (hasTrait(HeroTrait::Macguyver))
  {
    inventory.addFree(Item::CompressionSeal);
    inventory.addFree(Item::CompressionSeal);
    inventory.addFree(Item::TransmutationSeal);
    inventory.addFree(Item::TransmutationSeal);
    inventory.addFree(Item::TranslocationSeal);
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
  if (hasTrait(HeroTrait::DungeonLore))
    inventory.add(Spell::Lemmisi);
}

Hero::Hero(HeroStats stats, Defence defence, Experience experience)
  : name("Hero")
  , traits()
  , stats(std::move(stats))
  , defence(std::move(defence))
  , experience(std::move(experience))
  , inventory()
  , conversion(HeroClass::Guard, HeroRace::Human)
  , faith()
  , statuses()
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

MapResources
Hero::createResources(std::set<ResourceModifier> preparations, std::optional<God> preparedDeity, int mapSize) const
{
  if (hasTrait(HeroTrait::Hoarder))
    preparations.insert(ResourceModifier::Hoarder);
  if (hasTrait(HeroTrait::Martyr))
    preparations.insert(ResourceModifier::Martyr);
  if (hasTrait(HeroTrait::Merchant))
    preparations.insert(ResourceModifier::Merchant);
  return MapResources{EmptyResources{}, {preparations, preparedDeity, mapSize}, mapSize};
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

void Hero::gainExperienceForKill(int monsterLevel, bool monsterWasSlowed, Monsters& allMonsters)
{
  const int xpBase = Experience::forHeroAndMonsterLevels(getLevel(), monsterLevel);
  int xpBonuses = getStatusIntensity(HeroStatus::Learning);
  if (monsterWasSlowed)
    ++xpBonuses;
  if (hasTrait(HeroTrait::Veteran))
    ++xpBonuses;
  if (has(Item::BalancedDagger) && getLevel() == monsterLevel)
    xpBonuses += 2;
  gainExperience(xpBase, xpBonuses, allMonsters);
}

void Hero::gainExperienceForPetrification(bool monsterWasSlowed, Monsters& allMonsters)
{
  if (monsterWasSlowed)
    gainExperience(0, 1, allMonsters);
}

void Hero::gainExperienceNoBonuses(int xpGained, Monsters& allMonsters)
{
  gainExperience(0, xpGained, allMonsters);
}

void Hero::gainExperience(int xpBase, int xpBonuses, Monsters& allMonsters)
{
  int level = getLevel();
  const int prestige = getPrestige();
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
  const int initialLevel = getLevel();
  experience.gainLevel();
  if (getLevel() > initialLevel)
    levelGainedUpdate(getLevel(), allMonsters);
  levelUpRefresh(allMonsters);
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
  resetStatus(HeroDebuff::Poisoned);
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
  resetStatus(HeroDebuff::ManaBurned);
  if (hasTrait(HeroTrait::Courageous))
    addStatus(HeroStatus::Might);
  if (hasTrait(HeroTrait::Survivor))
    stats.healHitPoints(getHitPointsMax() * 2 / 10, false);
}

int Hero::nagaCauldronBonus() const
{
  return 5 * (static_cast<int>(hasStatus(HeroDebuff::Poisoned)) + static_cast<int>(hasStatus(HeroDebuff::ManaBurned)) +
              static_cast<int>(hasStatus(HeroDebuff::Corroded)) + static_cast<int>(hasStatus(HeroDebuff::Weakened)) +
              static_cast<int>(hasStatus(HeroDebuff::Cursed)));
}

int Hero::getBaseDamage() const
{
  const int modifiers = getStatusIntensity(HeroStatus::SpiritStrength) - getStatusIntensity(HeroDebuff::Weakened);
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

DamageType Hero::damageType() const
{
  return doesMagicalDamage() ? DamageType::Magical : DamageType::Physical;
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

int Hero::predictDamageTaken(int attackerDamageOutput, DamageType damageType) const
{
  return defence.predictDamageTaken(std::max(0, attackerDamageOutput - getStatusIntensity(HeroStatus::DamageReduction)),
                                    damageType, 0);
}

void Hero::loseHitPoints(int amountPointsLost, Monsters& allMonsters)
{
  stats.loseHitPointsWithoutDeathProtection(amountPointsLost);
  if (stats.getHitPoints() == 0 && hasStatus(HeroStatus::DeathProtection))
  {
    stats.barelySurvive();
    resetStatus(HeroStatus::DeathProtection);
    applyOrCollect(faith.deathProtectionTriggered(), allMonsters);
  }
}

bool Hero::takeDamage(int attackerDamageOutput, DamageType damageType, Monsters& allMonsters)
{
  const int damagePoints = predictDamageTaken(attackerDamageOutput, damageType);
  loseHitPoints(damagePoints, allMonsters);
  if (damagePoints > 0 && hasStatus(HeroStatus::Schadenfreude))
  {
    recoverManaPoints(damagePoints);
    resetStatus(HeroStatus::Schadenfreude);
  }
  resetStatus(HeroStatus::StoneSkin);
  return damagePoints > 0;
}

void Hero::recover(int nSquares)
{
  const bool exhausted = hasStatus(HeroStatus::Exhausted);
  if (!hasStatus(HeroDebuff::Poisoned))
    stats.healHitPoints(nSquares * recoveryMultiplier(), false);
  if (!hasStatus(HeroDebuff::ManaBurned) && !exhausted)
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
  // TODO: For Goatperson, never return a number larger than the amount of food in inventory!

  int numHP = 0;
  if (!hasStatus(HeroDebuff::Poisoned))
  {
    const int multiplier = recoveryMultiplier();
    numHP = (getHitPointsMax() - getHitPoints() + (multiplier - 1) /* always round up */) / multiplier;
  }
  const int numMP = hasStatus(HeroDebuff::ManaBurned) ? 0 : getManaPointsMax() - getManaPoints();
  if (hasTrait(HeroTrait::Damned))
    return numHP + numMP;
  return std::max(numHP, numMP);
}

void Hero::healHitPoints(int amountPointsHealed, bool mayOverheal)
{
  stats.healHitPoints(amountPointsHealed, mayOverheal);
}

void Hero::loseHitPointsOutsideOfFight(int amountPointsLost, Monsters& allMonsters)
{
  loseHitPoints(amountPointsLost, allMonsters);
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

void Hero::addSpiritStrength()
{
  const int mp = getManaPoints();
  const int newSpiritStrength = getLevel() + mp;
  auto iter = statuses.find(HeroStatus::SpiritStrength);
  if (iter == end(statuses))
    statuses[HeroStatus::SpiritStrength] = newSpiritStrength;
  else
    iter->second = std::max(newSpiritStrength, iter->second);
  loseManaPoints(mp);
}

void Hero::addStatus(HeroStatus status, int addedIntensity)
{
  setStatusIntensity(status, statuses[status] + addedIntensity);
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

void Hero::setStatusIntensity(HeroStatus status, int newIntensity)
{
  assert(status != HeroStatus::Exhausted && "Exhausted status is computed on the fly");
  const bool canStack = canHaveMultiple(status) || (status == HeroStatus::Might && hasTrait(HeroTrait::Additives));
  if (newIntensity > 1 && !canStack)
    newIntensity = 1;
  else if (newIntensity < 0)
    newIntensity = 0;

  const int oldIntensity = std::exchange(statuses[status], newIntensity);
  if (newIntensity == oldIntensity)
    return;

  if (status == HeroStatus::Momentum)
    changeDamageBonusPercent(newIntensity - oldIntensity);

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
    defence.setStoneSkin(newIntensity);
}

int Hero::getStatusIntensity(HeroStatus status) const
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

  int& intensity = debuffs[debuff];

  // Mana burn is special: Although one cannot have multiple layers, adding it will always set mana to 0
  if (debuff == HeroDebuff::ManaBurned)
  {
    PietyChange pietyChange;
    if (intensity == 0)
      pietyChange += faith.becameManaBurned();
    const int mp = getManaPoints();
    pietyChange += faith.manaPointsBurned(mp);
    loseManaPoints(mp);
    applyOrCollect(pietyChange, allMonsters);
  }

  if (intensity != 0 && !canHaveMultiple(debuff))
    return;

  intensity += addedIntensity;

  if (debuff == HeroDebuff::Poisoned)
    applyOrCollect(faith.becamePoisoned(), allMonsters);
  else if (debuff == HeroDebuff::Corroded)
    defence.setCorrosion(intensity);
  else if (debuff == HeroDebuff::Cursed)
    defence.setCursed(true);
}

void Hero::reduceStatus(HeroDebuff debuff)
{
  auto iter = debuffs.find(debuff);
  if (iter == debuffs.end() || iter->second == 0)
    return;

  const int newIntensity = iter->second - 1;

  if (newIntensity == 0)
  {
    resetStatus(debuff);
    return;
  }

  debuffs[debuff] = newIntensity;

  if (debuff == HeroDebuff::Corroded)
    defence.setCorrosion(newIntensity);
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

int Hero::getStatusIntensity(HeroDebuff debuff) const
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
  addStatus(HeroDebuff::Cursed, allMonsters, monster.bearsCurse() ? 1 : -1);
  applyOrCollect(faith.monsterKilled(monster, getLevel(), monsterWasBurning), allMonsters);
  gainExperienceForKill(monster.getLevel(), monsterWasSlowed, allMonsters);
  if (has(Item::GlovesOfMidas))
    ++inventory.gold;
  if (has(Item::StoneSigil))
    faith.gainPiety(1);
  if (has(Item::BlueBead) && !hasStatus(HeroDebuff::ManaBurned))
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
  resetStatus(HeroStatus::ConsecratedStrike);
  resetStatus(HeroStatus::CrushingBlow);
  resetStatus(HeroStatus::Might);
  resetStatus(HeroStatus::SpiritStrength);
  resetStatus(HeroStatus::FirstStrikeTemporary);
  resetStatus(HeroStatus::Reflexes);

  if (inventory.triswordUsed())
    changeBaseDamage(-1);
}

void Hero::levelGainedUpdate(int newLevel, Monsters& allMonsters)
{
  stats.setHitPointsMax(stats.getHitPointsMax() + 10 + stats.getHealthBonus());
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
  if (has(Item::Platemail))
    addStatus(HeroStatus::DamageReduction, 2);
  if (has(Item::MartyrWraps))
  {
    addStatus(HeroDebuff::Corroded, allMonsters);
    for (auto& monster : allMonsters)
      monster.corrode();
  }
  if (has(Item::MagePlate) && newLevel % 2 == 1)
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

  if (has(Item::PatchesTheTeddy) && !hasStatus(HeroStatus::Pessimist))
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

void Hero::addDodgeChancePercent(int percent, bool isPermanent)
{
  const auto statusToUpdate = isPermanent ? HeroStatus::DodgePermanent : HeroStatus::DodgeTemporary;
  const int newDodgeChance = std::min(std::max(getStatusIntensity(statusToUpdate) + percent, 0), 100);
  setStatusIntensity(statusToUpdate, newDodgeChance);
}

int Hero::getDodgeChancePercent() const
{
  const int dodgeChance =
      std::min(getStatusIntensity(HeroStatus::DodgePermanent) + getStatusIntensity(HeroStatus::DodgeTemporary), 100);
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
  if (has(Item::RockHeart))
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
  const int sanguine = getStatusIntensity(HeroStatus::Sanguine);
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
  const int manaCosts = Magic::spellCosts(Spell::Imawal, *this);
  if (getManaPoints() < manaCosts)
    return false;
  loseManaPoints(manaCosts);
  applyOrCollect(faith.imawalPetrifyPlant(manaCosts), allMonsters);
  return true;
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

void Hero::collectGoldPile()
{
  int amount = std::uniform_int_distribution<>(1, 3)(generator);
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

bool Hero::buy(Item item)
{
  if (!hasRoomFor(item) || !spendGold(buyingPrice(item)))
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

int Hero::getPiety() const
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

int Hero::receivedBoonCount(Boon boon) const
{
  return faith.boonCount(boon);
}

int Hero::getBoonCosts(Boon boon) const
{
  return faith.getCosts(boon, *this);
}

bool Hero::followDeity(God god, int numRevealedTiles)
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
  faith.desecrate(altar, *this, allMonsters, has(Item::AgnosticCollar));
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

void Hero::applyOrCollectPietyGain(int pointsGained)
{
  assert(pointsGained >= 0);
  if (collectedPiety)
    *collectedPiety += pointsGained;
  else
    faith.gainPiety(pointsGained);
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

void Hero::addConversionPoints(int points, Monsters& allMonsters)
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
  const int enchantedBeads = inventory.enchantPrayerBeads();
  changeHitPointsMax(enchantedBeads);
  changeDamageBonusPercent(+enchantedBeads);
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
    if (auto item = std::get_if<Item>(&entry.itemOrSpell); item)
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

bool Hero::canAfford(Item item) const
{
  return gold() >= buyingPrice(item);
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
    if (item && (*item == Item::Skullpicker || *item == Item::Wereward || *item == Item::Gloat || *item == Item::Will))
      faith.convertedTaurogItem(*this, allMonsters);
  }
}

bool Hero::canConvert(ItemOrSpell itemOrSpell) const
{
  return inventory.canConvert(itemOrSpell);
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

  case Item::OrbOfZot:
  case Item::WickedGuitar:
    return true;

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

void Hero::use(Item item, Monsters& allMonsters)
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
    gainExperienceNoBonuses(50, allMonsters);
    consumed = true;
    break;

  case Item::OrbOfZot:
    // TODO: Only apply to visible monsters
    for (auto& monster : allMonsters)
      monster.zot();
    consumed = true;
    break;
  case Item::WickedGuitar:
    // TODO: Only apply to visible monsters
    for (auto& monster : allMonsters)
      monster.makeWickedSick();
    break;

  // Potions
  case Item::HealthPotion:
    drinkHealthPotion();
    break;
  case Item::ManaPotion:
    drinkManaPotion();
    break;
  case Item::FortitudeTonic:
    resetStatus(HeroDebuff::Poisoned);
    resetStatus(HeroDebuff::Weakened);
    break;
  case Item::BurnSalve:
    resetStatus(HeroDebuff::ManaBurned);
    resetStatus(HeroDebuff::Corroded);
    break;
  case Item::StrengthPotion:
    addSpiritStrength();
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

  applyOrCollect(faith.itemUsed(item), allMonsters);

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

  if (consumed && inventory.remove(item))
    changeStatsFromItem(item, false);
}

bool Hero::canUse(Item item, const Monster& monster) const
{
  if (item == Item::SlayerWand)
    return !monster.isDefeated() && monster.getLevel() < 10;
  return canUse(item);
}

void Hero::use(Item item, Monster& monster, Monsters& allMonsters)
{
  if (item == Item::SlayerWand)
  {
    gainExperienceForKill(std::min(getLevel(), monster.getLevel()), monster.isSlowed(), allMonsters);
    monster.die();
    inventory.remove(item);
  }
}

bool Hero::useCompressionSealOn(ItemOrSpell itemOrSpell)
{
  if (has(Item::CompressionSeal) && inventory.compress(itemOrSpell))
  {
    lose(Item::CompressionSeal);
    return true;
  }
  return false;
}

bool Hero::useTransmutationSealOn(ItemOrSpell itemOrSpell, Monsters& allMonsters)
{
  if (has(Item::TransmutationSeal) && inventory.transmute(itemOrSpell))
  {
    inventory.remove(Item::TransmutationSeal);
    if (const auto item = std::get_if<Item>(&itemOrSpell))
    {
      changeStatsFromItem(*item, false);
      // Taurog doesn't tolerate transmuting one of his boons
      if (*item == Item::Skullpicker || *item == Item::Wereward || *item == Item::Gloat || *item == Item::Will)
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
  if (has(Item::TransmutationSeal))
  {
    lose(Item::TransmutationSeal);
    addGold(hasTrait(HeroTrait::BlackMarket) ? 11 : 10);
    return true;
  }
  return false;
}

bool Hero::useTranslocationSealOn(Item shopItem)
{
  if (has(Item::TranslocationSeal))
  {
    lose(Item::TranslocationSeal);
    return inventory.translocate(shopItem);
  }
  return false;
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
    // Was not removed when coverting item in early versions of the game.
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
      resetStatus(HeroStatus::PoisonImmune);
    break;
  case Item::SoulOrb:
    if (itemReceived)
      addStatus(HeroStatus::ManaBurnImmune);
    else if (!hasTrait(HeroTrait::Scars) && !hasTrait(HeroTrait::Undead))
      resetStatus(HeroStatus::ManaBurnImmune);
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
      resetStatus(HeroStatus::DeathGazeImmune);
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
  const int indulgence = hero.getFaith().getIndulgence();
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
