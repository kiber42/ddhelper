#include "engine/Faith.hpp"

#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

#include <algorithm>
#include <cassert>

PietyChange::PietyChange(int deltaPoints)
  : values({deltaPoints})
  , pact(std::nullopt)
  , jehora(false)
{
}

PietyChange::PietyChange(Pact activated)
  : values()
  , pact(activated)
  , jehora(false)
{
}

PietyChange::PietyChange(JehoraTriggered)
  : values()
  , pact(std::nullopt)
  , jehora(true)
{
}

const std::vector<int>& PietyChange::operator()() const
{
  return values;
}

std::optional<Pact> PietyChange::activatedPact() const
{
  return pact;
}

bool PietyChange::randomJehoraEvent() const
{
  return jehora;
}

PietyChange& PietyChange::operator+=(const PietyChange& other)
{
  for (auto otherValue : other.values)
    values.emplace_back(otherValue);
  if (!pact.has_value())
    pact = other.pact;
  jehora |= other.jehora;
  return *this;
}

PietyChange& PietyChange::operator+=(int deltaPoints)
{
  values.emplace_back(deltaPoints);
  return *this;
}

bool Faith::followDeity(God god, Hero& hero, int numRevealedTiles)
{
  if (!canFollow(god, hero))
    return false;
  if (followedDeity)
  {
    piety /= 2;
    numConsecutiveLevelUpsWithGlowingGuardian = 0;
  }
  else
    initialBoon(god, hero, numRevealedTiles);
  followedDeity = god;
  return true;
}

bool Faith::canFollow(God god, const Hero& hero) const
{
  if (hero.hasTrait(HeroTrait::Damned) || hero.hasTrait(HeroTrait::Scapegoat))
    return false;
  if (followedDeity)
  {
    if (followedDeity == god)
      return false;
    if (hero.hasTrait(HeroTrait::HolyWork))
      return false;
    if (piety < 50)
      return false;
  }
  return true;
}

std::optional<God> Faith::getFollowedDeity() const
{
  return followedDeity;
}

int Faith::getPiety() const
{
  return piety;
}

int Faith::getMaxPiety() const
{
  return consensus ? 50 : 100;
}

bool Faith::has(Boon boon) const
{
  return std::find(begin(boons), end(boons), boon) != end(boons);
}

int Faith::boonCount(Boon boon) const
{
  if (!allowRepeatedUse(boon))
    return (int)(has(boon));
  return std::count(begin(boons), end(boons), boon);
}

int Faith::getIndulgence() const
{
  return indulgence;
}

void Faith::gainPiety(int pointsGained)
{
  if (pointsGained > 0)
    piety = std::min(piety + pointsGained, getMaxPiety());
}

void Faith::losePiety(int pointsLost, Hero& hero, Monsters& allMonsters)
{
  if (pointsLost <= 0)
    return;
  if (indulgence > 0)
  {
    --indulgence;
    return;
  }
  piety -= pointsLost;
  if (piety < 0)
  {
    piety = 0;
    assert(followedDeity.has_value());
    punish(*followedDeity, hero, allMonsters);
  }
}

void Faith::applyRandomJehoraEvent(Hero& hero)
{
  const int result = jehora();
  if (result > 0 && !hero.hasStatus(HeroStatus::Pessimist))
    gainPiety(result);
  else
    jehora.applyRandomPunishment(hero);
}

void Faith::apply(PietyChange change, Hero& hero, Monsters& allMonsters)
{
  for (int value : change())
  {
    if (value > 0)
      gainPiety(value);
    else if (value < 0)
      losePiety(-value, hero, allMonsters);
  }
  if (change.randomJehoraEvent())
    applyRandomJehoraEvent(hero);
  const auto pact = change.activatedPact();
  if (pact)
  {
    switch (*pact)
    {
    case Pact::ScholarsPact:
      if (piety >= 10)
      {
        piety -= 10;
        hero.addStatus(HeroStatus::Might);
        hero.healHitPoints(hero.getHitPointsMax(), true);
      }
      break;
    case Pact::WarriorsPact:
      if (piety >= 3)
      {
        piety -= 3;
        hero.changeHitPointsMax(+1);
      }
      break;
    case Pact::AlchemistsPact:
      if (piety >= 4)
      {
        piety -= 4;
        hero.gainExperienceNoBonuses(3, allMonsters);
      }
      break;
    case Pact::BodyPact:
      if (piety >= 4)
      {
        piety -= 4;
        hero.changePhysicalResistPercent(1);
        hero.changeMagicalResistPercent(1);
      }
      break;
    case Pact::SpiritPact:
      if (piety >= 5)
      {
        piety -= 5;
        hero.addConversionPoints(15, allMonsters);
      }
      break;
    case Pact::Consensus:
      break;
    }
  }
}

int Faith::isAvailable(Boon boon, const Hero& hero, const Monsters& allMonsters, const Resources& resources) const
{
  if (deity(boon) != followedDeity)
    return false;
  if (!allowRepeatedUse(boon) && boonCount(boon) > 0)
    return false;
  const int costs = getCosts(boon, hero);
  if (costs > piety)
    return false;
  switch (boon)
  {
  case Boon::Absolution:
    return glowingGuardian.canUseAbsolution(hero.getLevel(), allMonsters);
  case Boon::BloodCurse:
    return hero.getLevel() < 10;
  case Boon::BoostHealth:
    return hero.has(Potion::HealthPotion);
  case Boon::BoostMana:
    return hero.has(Potion::ManaPotion);
  case Boon::Humility:
    return hero.getLevel() > 1;
  case Boon::Reflexes:
    return hero.has(Potion::HealthPotion);
  case Boon::StoneFist:
    return resources().numWalls >= 20;
  case Boon::StoneForm:
    return resources().numWalls >= 10;
  case Boon::StoneHeart:
    return resources().numWalls >= 15;
  case Boon::StoneSkin:
    return resources().numWalls >= 3;
  case Boon::Tribute:
    return hero.gold() >= 15;
  case Boon::UnstoppableFury:
    return !hero.hasStatus(HeroStatus::DeathProtection) && hero.has(TaurogItem::Skullpicker) &&
           hero.has(TaurogItem::Wereward) && hero.has(TaurogItem::Gloat) && hero.has(TaurogItem::Will);
  default:
    return true;
  }
}

bool Faith::request(Boon boon, Hero& hero, Monsters& allMonstersOnFloor, Resources& resources)
{
  if (!isAvailable(boon, hero, allMonstersOnFloor, resources))
    return false;
  const int costs = getCosts(boon, hero);
  piety -= costs;
  boons.push_back(boon);
  // Apply immediate effects.
  switch (boon)
  {
  case Boon::StoneSoup:
    hero.receiveFreeSpell(Spell::Endiswal);
    break;
  case Boon::StoneSkin:
    resources().numWalls -= 3;
    if (hero.getFaith().has(Boon::StoneForm))
      hero.addStatus(HeroStatus::Might);
    hero.addStatus(HeroStatus::StoneSkin, 3);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 3);
    break;
  case Boon::StoneForm:
    resources().numWalls -= 10;
    hero.addStatus(HeroStatus::Might);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 5);
    break;
  case Boon::StoneFist:
    resources().numWalls -= 20;
    if (hero.getFaith().has(Boon::StoneForm))
      hero.addStatus(HeroStatus::Might);
    hero.addStatus(HeroStatus::Knockback, 50);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 5);
    break;
  case Boon::StoneHeart:
    resources().numWalls -= 15;
    if (hero.getFaith().has(Boon::StoneForm))
      hero.addStatus(HeroStatus::Might);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 3);
    for (auto& monster : allMonstersOnFloor)
      monster.addMagicResist(-5);
    break;

  case Boon::BloodCurse:
    hero.modifyLevelBy(+1);
    break;
  case Boon::BloodTithe:
    hero.addStatus(HeroStatus::Sanguine, 5);
    hero.changeHitPointsMax(-5);
    hero.changeBaseDamage(+1);
    break;
  case Boon::BloodHunger:
    hero.addStatus(HeroStatus::LifeSteal);
    hero.changePhysicalResistPercentMax(-20);
    hero.changeMagicalResistPercentMax(-20);
    break;
  case Boon::BloodShield:
    hero.changePhysicalResistPercent(+15);
    hero.changeMagicalResistPercent(+15);
    break;
  case Boon::BloodSwell:
    hero.healHitPoints(hero.getHitPointsMax());
    hero.addStatus(HeroDebuff::Cursed, allMonstersOnFloor);
    break;

  case Boon::Plantation:
  {
    auto& resourceSet = resources();
    gainPiety(5 * resourceSet.numBloodPools);
    resourceSet.numPlants += resourceSet.numBloodPools;
    resourceSet.numBloodPools = 0;
    break;
  }
  case Boon::Clearance:
  {
    auto& resourceSet = resources();
    const int cleared = std::min(10, resourceSet.numPlants);
    hero.recoverManaPoints(cleared);
    resourceSet.numPlants -= cleared;
    break;
  }
  case Boon::Greenblood:
    resources().numPlants += 3;
    hero.reduceStatus(HeroDebuff::Cursed);
    for (auto& monster : allMonstersOnFloor)
      monster.corrode();
    break;
  case Boon::Entanglement:
    resources().numPlants += 5;
    for (auto& monster : allMonstersOnFloor)
      monster.slow();
    break;
  case Boon::VineForm:
    resources().numPlants += 2;
    hero.changeHitPointsMax(+4);
    hero.addStatus(HeroStatus::DamageReduction);
    break;

  case Boon::Humility:
    hero.modifyLevelBy(-1);
    break;
  case Boon::Absolution:
  {
    // Precondition: Removes one monster of <= hero's level (except undead, bosses, other dungeon levels)
    auto monsterIt = glowingGuardian.pickMonsterForAbsolution(hero.getLevel(), allMonstersOnFloor);
    if (monsterIt != end(allMonstersOnFloor))
    {
      allMonstersOnFloor.erase(monsterIt);
      hero.changeHitPointsMax(4);
      hero.receive(MiscItem::PrayerBead);
    }
    break;
  }
  case Boon::Cleansing:
    hero.resetStatus(HeroDebuff::Poisoned);
    hero.resetStatus(HeroDebuff::ManaBurned);
    hero.reduceStatus(HeroDebuff::Weakened);
    hero.reduceStatus(HeroDebuff::Corroded);
    hero.addStatus(HeroStatus::ConsecratedStrike);
    hero.receive(MiscItem::PrayerBead);
    break;
  case Boon::Protection:
    hero.healHitPoints(hero.getHitPointsMax() * 35 / 100);
    hero.recoverManaPoints(hero.getManaPointsMax() * 35 / 100);
    hero.receive(MiscItem::PrayerBead);
    break;
  case Boon::Enlightenment:
  {
    hero.receiveEnlightenment(allMonstersOnFloor);
    break;
  }

  case Boon::LastChance:
    if (jehora.lastChanceSuccessful(costs))
      hero.refillHealthAndMana();
    break;
  case Boon::BoostHealth:
    hero.lose(Potion::HealthPotion);
    hero.changeHitPointsMax(+20);
    break;
  case Boon::BoostMana:
    hero.lose(Potion::ManaPotion);
    hero.changeManaPointsMax(+3);
    break;
  case Boon::ChaosAvatar:
    hero.gainLevel(allMonstersOnFloor);
    hero.refillHealthAndMana(); // for Goatperson
    hero.addConversionPoints(100, allMonstersOnFloor);
    hero.resetStatus(HeroDebuff::Weakened);
    hero.resetStatus(HeroDebuff::Corroded);
    for (auto& monster : allMonstersOnFloor)
    {
      monster.addPhysicalResist(-20);
      monster.addMagicResist(-20);
    }
    break;

  case Boon::Magic:
    hero.changeManaPointsMax(+1);
    break;
  case Boon::Flames:
    hero.changeDamageBonusPercent(-50);
    break;
  case Boon::Weakening:
    hero.changeMagicalResistPercent(-10);
    for (auto& monster : allMonstersOnFloor)
      monster.addMagicResist(-10);
    break;

  case Boon::TaurogsBlade:
    hero.receive(TaurogItem::Skullpicker);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsShield:
    hero.receive(TaurogItem::Wereward);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsHelm:
    hero.receive(TaurogItem::Gloat);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsArmour:
    hero.receive(TaurogItem::Will);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::UnstoppableFury:
    hero.addStatus(HeroStatus::DeathProtection);
    break;

  case Boon::Tribute:
    hero.spendGold(15);
    break;
  case Boon::TikkisEdge:
    hero.addStatus(HeroStatus::Learning);
    hero.addGold(10);
    break;
  case Boon::Dodging:
    hero.addDodgeChancePercent(10, true);
    hero.addGold(10);
    break;
  case Boon::Poison:
    hero.addStatus(HeroStatus::Poisonous, 1);
    break;
  case Boon::Reflexes:
    hero.lose(Potion::HealthPotion);
    hero.receive(Potion::ReflexPotion);
    hero.receive(Potion::QuicksilverPotion);
    break;

  // No immediate effects
  case Boon::Petition:
  case Boon::Refreshment:
  case Boon::MysticBalance:
    break;
  }
  return true;
}

int Faith::getCosts(Boon boon, const Hero& hero) const
{
  const auto [baseCosts, increase] = [boon]() -> std::pair<int, int> {
    switch (boon)
    {
    case Boon::StoneSoup:
      return {35, 0};
    case Boon::StoneSkin:
      return {15, 0};
    case Boon::StoneForm:
      return {25, 0};
    case Boon::StoneFist:
      return {40, 0};
    case Boon::StoneHeart:
      return {10, 0};

    case Boon::BloodCurse:
      return {-20, 0};
    case Boon::BloodTithe:
      return {10, 15};
    case Boon::BloodHunger:
      return {20, 25};
    case Boon::BloodShield:
      return {40, 0};
    case Boon::BloodSwell:
      return {20, 10};

    case Boon::Plantation:
      return {5, 0};
    case Boon::Clearance:
      return {5, 5};
    case Boon::Greenblood:
      return {5, 3};
    case Boon::Entanglement:
      return {5, 0};
    case Boon::VineForm:
      return {5, 3};

    case Boon::Humility:
      return {15, 0};
    case Boon::Absolution:
      return {2, 2};
    case Boon::Cleansing:
      return {10, 0};
    case Boon::Protection:
      return {10, 5};
    case Boon::Enlightenment:
      return {100, 0};

    case Boon::Petition:
      return {45, 0};
    case Boon::LastChance:
      return {-1, 0};
    case Boon::BoostHealth:
      return {20, 25};
    case Boon::BoostMana:
      return {20, 25};
    case Boon::ChaosAvatar:
      return {80, 0};

    case Boon::Magic:
      return {5, 20};
    case Boon::Refreshment:
      return {50, 0};
    case Boon::Flames:
      return {20, 0};
    case Boon::Weakening:
      return {30, 0};
    case Boon::MysticBalance:
      return {60, 0};

    case Boon::TaurogsBlade:
      return {20, 0};
    case Boon::TaurogsShield:
      return {25, 0};
    case Boon::TaurogsHelm:
      return {25, 0};
    case Boon::TaurogsArmour:
      return {25, 0};
    case Boon::UnstoppableFury:
      return {20, 10};

    case Boon::Tribute:
      return {-10, 0};
    case Boon::TikkisEdge:
      return {25, 25};
    case Boon::Dodging:
      return {25, 0};
    case Boon::Poison:
      return {15, 10};
    case Boon::Reflexes:
      return {35, 0};
    }
  }();
  // Last Chance uses all remaining piety points
  if (baseCosts == -1)
    return getPiety();
  const int count = boonCount(boon);
  if (hero.hasTrait(HeroTrait::HolyWork))
    return baseCosts * 8 / 10 + (increase * 8 / 10) * count;
  return baseCosts + increase * count;
}

std::optional<Pact> Faith::getPact() const
{
  return pact;
}

bool Faith::enter(Pact newPact)
{
  if (pact)
    return false;
  if (newPact == Pact::Consensus)
  {
    if (consensus)
      return false;
    consensus = true;
    gainPiety(50);
  }
  else
    pact = newPact;
  return true;
}

bool Faith::enteredConsensus() const
{
  return consensus;
}

void Faith::initialBoon(God god, Hero& hero, int numRevealedTiles)
{
  switch (god)
  {
  case God::BinlorIronshield:
    hero.receiveFreeSpell(Spell::Pisorf);
    gainPiety(numRevealedTiles / 10);
    break;
  case God::Dracul:
    gainPiety(2 * numMonstersKilled);
    break;
  case God::TheEarthmother:
    gainPiety(5);
    hero.receiveFreeSpell(Spell::Imawal);
    break;
  case God::GlowingGuardian:
    gainPiety(5 * hero.getLevel());
    break;
  case God::JehoraJeheyu:
    gainPiety(jehora.initialPietyBonus(hero.getLevel(), hero.hasStatus(HeroStatus::Pessimist)));
    hero.receiveFreeSpell(Spell::Weytwut);
    break;
  case God::MysteraAnnur:
    gainPiety(numSpellsCast);
    break;
  case God::Taurog:
    gainPiety(2 * numMonstersKilled);
    break;
  case God::TikkiTooki:
    hero.receiveFreeSpell(Spell::Getindare);
    break;
  }
}

void Faith::punish(God god, Hero& hero, Monsters& allMonsters)
{
  if (hero.hasTrait(HeroTrait::HolyWork))
    return;
  switch (god)
  {
  case God::BinlorIronshield:
    hero.changePhysicalResistPercent(-50);
    hero.changeMagicalResistPercent(-50);
    break;
  case God::Dracul:
    hero.reduceStatus(HeroStatus::LifeSteal);
    hero.changeHitPointsMax(-20);
    break;
  case God::TheEarthmother:
    hero.addStatus(HeroDebuff::Corroded, allMonsters, 5);
    break;
  case God::GlowingGuardian:
    hero.clearInventory();
    break;
  case God::JehoraJeheyu:
    // TODO: 50% chance
    hero.loseHitPointsOutsideOfFight(hero.getHitPoints() - 1, allMonsters);
    hero.loseManaPoints(hero.getManaPoints());
    hero.setHitPointsMax(hero.getHitPointsMax() * 2 / 3);
    hero.setManaPointsMax(hero.getManaPointsMax() * 2 / 3);
    hero.changeDamageBonusPercent(-33);
    break;
  case God::MysteraAnnur:
    for (auto& monster : allMonsters)
    {
      monster.addPhysicalResist(15);
      monster.addMagicResist(15);
    }
    break;
  case God::Taurog:
    hero.changeDamageBonusPercent(-40);
    for (auto& monster : allMonsters)
      monster.addMagicResist(10);
    break;
  case God::TikkiTooki:
    hero.resetStatus(HeroStatus::DodgePermanent);
    hero.resetStatus(HeroStatus::DodgeTemporary);
    hero.resetStatus(HeroStatus::Poisonous);
    for (auto& monster : allMonsters)
    {
      monster.makeFast();
      monster.makeWeakening();
    }
    break;
  }
}

void Faith::desecrate(God altar, Hero& hero, Monsters& allMonsters, bool hasAgnosticCollar)
{
  if (!hasAgnosticCollar)
    punish(altar, hero, allMonsters);
  if (numDesecrated < 3)
    gainPiety((3 - numDesecrated) * 10);
  ++numDesecrated;
  indulgence += 3;
}

PietyChange Faith::monsterKilled(const Monster& monster, int heroLevel, bool monsterWasBurning)
{
  ++numMonstersKilled;

  PietyChange result;
  if (pact == Pact::WarriorsPact && monster.grantsXP())
    result += *pact;
  if (followedDeity)
  {
    result += [&, deity = *followedDeity]() -> PietyChange {
      switch (deity)
      {
      case God::Dracul:
        return monster.isUndead() ? -5 : 2;
      case God::GlowingGuardian:
      {
        int award = 0;
        if (monster.isUndead() && monster.grantsXP())
          ++award;
        if (monsterWasBurning)
          ++award;
        if (award > 0)
          return award;
      }
      break;
      case God::JehoraJeheyu:
        if (monster.grantsXP())
          return JehoraTriggered{};
        break;
      case God::MysteraAnnur:
        if (monster.doesMagicalDamage())
          return -5;
        break;
      case God::Taurog:
        return monster.doesMagicalDamage() ? 8 : 4;
      case God::TikkiTooki:
        if (monster.grantsXP() && monster.getLevel() < heroLevel)
          return 5;
        break;
      default:
        break;
      }
      return {};
    }();
  }
  return result;
}

PietyChange Faith::monsterPoisoned(const Monster& monster)
{
  if (followedDeity == God::TikkiTooki)
  {
    auto& monsterHistory = history[monster.getID()];
    if (!monsterHistory.becamePoisoned)
    {
      monsterHistory.becamePoisoned = true;
      return 1;
    }
  }
  else if (followedDeity == God::GlowingGuardian)
    return -10;
  return {};
}

PietyChange Faith::spellCast(Spell spell, int manaCost)
{
  ++numSpellsCast;
  if (!followedDeity)
    return {};
  switch (*followedDeity)
  {
  case God::BinlorIronshield:
    if (spell == Spell::Bysseps)
      return 1;
    break;
  case God::Dracul:
    if (spell == Spell::Cydstepp || spell == Spell::Halpmeh)
      return -5;
    break;
  case God::TheEarthmother:
    if (spell == Spell::Imawal)
      return 10;
    break;
  case God::GlowingGuardian:
    if (spell == Spell::Bludtupowa)
      return -20;
    break;
  case God::JehoraJeheyu:
    if (spell == Spell::Apheelsik || spell == Spell::Bysseps || spell == Spell::Cydstepp || spell == Spell::Getindare ||
        spell == Spell::Weytwut || spell == Spell::Wonafyt)
      return JehoraTriggered{};
    break;
  case God::MysteraAnnur:
    numManaPointsSpent += manaCost;
    if (numManaPointsSpent >= 2)
    {
      const int reward = numManaPointsSpent / 2;
      numManaPointsSpent %= 2;
      return reward;
    }
    break;
  case God::Taurog:
    return -2;
  case God::TikkiTooki:
    if (spell == Spell::Weytwut || spell == Spell::Wonafyt)
      return 1;
    break;
  }
  return {};
}

PietyChange Faith::imawalCreateWall(int manaCost)
{
  if (followedDeity == God::BinlorIronshield)
    return -5;
  if (followedDeity == God::TheEarthmother)
    return {};
  return spellCast(Spell::Imawal, manaCost);
}

PietyChange Faith::imawalPetrifyPlant(int manaCost)
{
  if (followedDeity == God::TheEarthmother)
    return 5;
  return spellCast(Spell::Imawal, manaCost);
}

PietyChange Faith::levelGained()
{
  PietyChange result;
  if (pact == Pact::ScholarsPact)
    result += *pact;
  if (followedDeity)
  {
    result += [&, deity = *followedDeity]() -> PietyChange {
      if (deity == God::BinlorIronshield)
        return -10;
      if (deity == God::GlowingGuardian)
      {
        // TODO: Preparation penalty for Glowing Guardian: reduce level-up bonus to 2 * (N - 1)
        ++numConsecutiveLevelUpsWithGlowingGuardian;
        return 3 * numConsecutiveLevelUpsWithGlowingGuardian;
      }
      return {};
    }();
  }
  return result;
}

PietyChange Faith::itemUsed(Item item)
{
  PietyChange result;
  if (auto potion = std::get_if<Potion>(&item); potion && (*potion == Potion::HealthPotion || *potion == Potion::ManaPotion))
  {
    if (pact == Pact::AlchemistsPact)
      result += *pact;
    if (followedDeity)
    {
      result += [&, deity = *followedDeity]() -> PietyChange {
        if (*potion == Potion::HealthPotion)
        {
          if (deity == God::Dracul)
            return -5;
          if (deity == God::GlowingGuardian)
            return -10;
        }
        else if (*potion == Potion::ManaPotion)
        {
          if (deity == God::GlowingGuardian)
            return -10;
        }
        return {};
      }();
    }
  }
  return result;
}

PietyChange Faith::lifeStolen(const Monster& monster)
{
  if (followedDeity == God::Dracul && monster.grantsXP())
  {
    auto& monsterHistory = history[monster.getID()];
    if (!monsterHistory.hadLifeStolen)
    {
      monsterHistory.hadLifeStolen = true;
      return 1;
    }
  }
  else if (followedDeity == God::TheEarthmother)
    return -5;
  else if (followedDeity == God::GlowingGuardian)
    return -10;
  return {};
}

PietyChange Faith::bloodPoolConsumed(int numBloodTithe)
{
  if (followedDeity == God::Dracul && numBloodTithe > 0)
    return numBloodTithe;
  else if (followedDeity == God::GlowingGuardian)
    return -10;
  return {};
}

PietyChange Faith::becamePoisoned()
{
  if (followedDeity == God::GlowingGuardian)
    return 2;
  return {};
}

PietyChange Faith::becameManaBurned()
{
  if (followedDeity == God::GlowingGuardian)
    return 2;
  else if (followedDeity == God::MysteraAnnur)
    return -1;
  return {};
}

PietyChange Faith::manaPointsBurned(int pointsLost)
{
  if (followedDeity == God::MysteraAnnur)
    return -pointsLost;
  return {};
}

PietyChange Faith::converted(ItemOrSpell itemOrSpell, bool wasSmall)
{
  PietyChange result;
  if (pact == Pact::SpiritPact)
    result += *pact;
  if (followedDeity)
  {
    result += [&, deity = *followedDeity]() -> PietyChange {
      switch (deity)
      {
      case God::Dracul:
        if (itemOrSpell == ItemOrSpell{Potion::HealthPotion})
          return 5;
        if (itemOrSpell == ItemOrSpell{Spell::Cydstepp} || itemOrSpell == ItemOrSpell{Spell::Halpmeh})
          return 10;
        return {};
      case God::GlowingGuardian:
        if (itemOrSpell == ItemOrSpell{Spell::Apheelsik} || itemOrSpell == ItemOrSpell{Spell::Bludtupowa})
          return 10;
        if (!wasSmall)
          return 5;
        if (const auto item = std::get_if<Item>(&itemOrSpell); item && std::get_if<Potion>(&*item) != nullptr)
          return 5;
        return 2;
      case God::JehoraJeheyu:
        return JehoraTriggered();
      case God::Taurog:
        if (std::get_if<Spell>(&itemOrSpell) != nullptr)
          return 10;
        else
          return {};
      default:
        return {};
      }
    }();
  }
  return result;
}

PietyChange Faith::plantDestroyed()
{
  if (followedDeity == God::TheEarthmother)
    return -15;
  return {};
}

PietyChange Faith::receivedHit(const Monster& monster)
{
  PietyChange result;
  if (pact == Pact::BodyPact && monster.grantsXP())
    result += *pact;
  if (followedDeity == God::TikkiTooki)
  {
    // TODO: If Tikki Tooki was equipped, any hit results in a penalty
    auto& monsterHistory = history[monster.getID()];
    if (monsterHistory.hitHero)
      result += PietyChange{-3};
    monsterHistory.hitHero = true;
  }
  return result;
}

PietyChange Faith::dodgedAttack()
{
  if (followedDeity == God::JehoraJeheyu)
    return JehoraTriggered{};
  if (followedDeity == God::TikkiTooki)
    return 3;
  return {};
}

PietyChange Faith::deathProtectionTriggered()
{
  if (followedDeity == God::TikkiTooki)
    return -10;
  return {};
}

void Faith::convertedTaurogItem(Hero& hero, Monsters& allMonsters)
{
  if (hero.hasTrait(HeroTrait::HolyWork))
    return;
  hero.changeDamageBonusPercent(-10);
  for (auto& monster : allMonsters)
    monster.addMagicResist(10);
}
