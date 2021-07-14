#include "engine/Faith.hpp"

#include "engine/Clamp.hpp"
#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Spells.hpp"

#include <algorithm>
#include <cassert>
#include <variant>

Faith::Faith(std::optional<GodOrPactmaker> preparedAltar0)
{
  if (preparedAltar0)
  {
    if (auto god = std::get_if<God>(&*preparedAltar0))
      preparedAltar.emplace(*god);
  }
}

bool Faith::followDeity(God god, Hero& hero, unsigned numRevealedTiles, Resources& resources)
{
  if (!canFollow(god, hero))
    return false;
  if (followedDeity)
  {
    piety /= 2;
    numConsecutiveLevelUpsWithGlowingGuardian = 0;
  }
  else
    initialBoon(god, hero, numRevealedTiles, resources);
  followedDeity = god;
  return true;
}

void Faith::makeGoatperson(std::vector<God> altars)
{
  altarsForGoatperson = altars;
  selectNextDeityForGoatperson();
}

void Faith::selectNextDeityForGoatperson()
{
  assert(altarsForGoatperson);
  if (altarsForGoatperson->empty())
    return;
  if (followedDeity && altarsForGoatperson->size() == 1)
  {
    assert(*followedDeity == (*altarsForGoatperson)[0]);
    return;
  }
  std::uniform_int_distribution<size_t> randomIndex(0u, altarsForGoatperson->size());
  God lastDeity = *followedDeity;
  do
  {
    followedDeity = (*altarsForGoatperson)[randomIndex(generator)];
  } while (lastDeity == *followedDeity);
}

bool Faith::canFollow(God god, const Hero& hero) const
{
  if (altarsForGoatperson || hero.has(HeroTrait::Damned))
    return false;
  if (followedDeity)
  {
    if (followedDeity == god)
      return false;
    if (hero.has(HeroTrait::HolyWork))
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

unsigned Faith::getPiety() const
{
  return piety;
}

unsigned Faith::getMaxPiety() const
{
  return consensus ? 50u : 100u;
}

bool Faith::has(Boon boon) const
{
  return std::find(begin(boons), end(boons), boon) != end(boons);
}

unsigned Faith::boonCount(Boon boon) const
{
  return clampedTo<unsigned>(std::count(begin(boons), end(boons), boon));
}

unsigned Faith::getIndulgence() const
{
  return indulgence;
}

void Faith::gainPiety(unsigned pointsGained)
{
  piety = std::min(piety + pointsGained, getMaxPiety());
}

void Faith::losePiety(unsigned pointsLost, Hero& hero, Monsters& allMonsters)
{
  if (indulgence > 0 && pointsLost > 0)
  {
    --indulgence;
    return;
  }
  if (piety < pointsLost)
  {
    piety = 0;
    assert(followedDeity);
    punish(*followedDeity, hero, allMonsters);
  }
  else
  {
    piety -= pointsLost;
  }
}

void Faith::applyRandomJehoraEvent(Hero& hero)
{
  const auto result = jehora();
  if (result > 0 && !hero.has(HeroStatus::Pessimist))
    gainPiety(result);
  else
    jehora.applyRandomPunishment(hero);
}

void Faith::apply(PietyChange change, Hero& hero, Monsters& allMonsters)
{
  for (int value : change())
  {
    if (value > 0)
      gainPiety(static_cast<unsigned>(value));
    else if (value < 0)
      losePiety(static_cast<unsigned>(-value), hero, allMonsters);
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
        hero.add(HeroStatus::Might);
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
  if (costs > 0 && static_cast<unsigned>(costs) > piety)
    return false;
  switch (boon)
  {
  case Boon::Absolution:
    return hero.hasRoomFor(MiscItem::PrayerBead) && glowingGuardian.canUseAbsolution(hero.getLevel(), allMonsters);
  case Boon::BloodCurse:
    return hero.getLevel() < 10;
  case Boon::BoostHealth:
    return hero.has(Potion::HealthPotion);
  case Boon::BoostMana:
    return hero.has(Potion::ManaPotion);
  case Boon::Cleansing:
    return hero.hasRoomFor(MiscItem::PrayerBead);
  case Boon::Humility:
    return hero.getLevel() > 1;
  case Boon::Protection:
    return hero.hasRoomFor(MiscItem::PrayerBead);
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
    return !hero.has(HeroStatus::DeathProtection) && hero.has(TaurogItem::Skullpicker) &&
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
  const int newPiety = static_cast<int>(piety) - costs;
  if (newPiety < 0)
  {
    assert(newPiety >= 0);
  }
  piety = static_cast<unsigned>(newPiety);
  boons.push_back(boon);

  auto triggerStoneForm = [&] {
    if ((boon == Boon::StoneForm || hero.has(Boon::StoneForm)) && !hero.has(HeroStatus::Might))
      hero.add(HeroStatus::Might);
  };
  auto receive = [&](auto itemOrSpell) {
    if (!hero.receive(itemOrSpell))
      resources().onGround.push_back(itemOrSpell);
  };

  // Apply immediate effects.
  switch (boon)
  {
  case Boon::StoneSoup:
    if (!hero.receiveFreeSpell(Spell::Endiswal))
      resources().freeSpells.push_back(Spell::Endiswal);
    break;
  case Boon::StoneSkin:
    resources().numWalls -= 3;
    triggerStoneForm();
    hero.add(HeroStatus::StoneSkin, 3);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 3);
    break;
  case Boon::StoneForm:
    resources().numWalls -= 10;
    triggerStoneForm();
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 5);
    break;
  case Boon::StoneFist:
    resources().numWalls -= 20;
    triggerStoneForm();
    hero.add(HeroStatus::Knockback, 50);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 5);
    break;
  case Boon::StoneHeart:
    resources().numWalls -= 15;
    triggerStoneForm();
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 3);
    for (auto& monster : allMonstersOnFloor)
      monster.changeMagicResist(-5);
    break;

  case Boon::BloodCurse:
    hero.modifyLevelBy(+1);
    break;
  case Boon::BloodTithe:
    hero.add(HeroStatus::Sanguine, 5);
    hero.changeHitPointsMax(-5);
    hero.changeBaseDamage(+1);
    break;
  case Boon::BloodHunger:
    hero.add(HeroStatus::LifeSteal);
    hero.changePhysicalResistPercentMax(-20);
    hero.changeMagicalResistPercentMax(-20);
    break;
  case Boon::BloodShield:
    hero.changePhysicalResistPercent(+15);
    hero.changeMagicalResistPercent(+15);
    break;
  case Boon::BloodSwell:
    hero.healHitPoints(hero.getHitPointsMax());
    hero.add(HeroDebuff::Cursed, allMonstersOnFloor);
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
    const auto cleared = std::min(10u, resourceSet.numPlants);
    hero.recoverManaPoints(cleared);
    resourceSet.numPlants -= cleared;
    break;
  }
  case Boon::Greenblood:
    resources().numPlants += preparationPenaltyApplies() ? 6 : 3;
    hero.reduce(HeroDebuff::Cursed);
    for (auto& monster : allMonstersOnFloor)
      monster.corrode();
    break;
  case Boon::Entanglement:
    resources().numPlants += preparationPenaltyApplies() ? 10 : 5;
    for (auto& monster : allMonstersOnFloor)
      monster.slow();
    break;
  case Boon::VineForm:
    resources().numPlants += preparationPenaltyApplies() ? 4 : 2;
    hero.changeHitPointsMax(+4);
    hero.add(HeroStatus::DamageReduction);
    break;

  case Boon::Humility:
    hero.modifyLevelBy(-1);
    break;
  case Boon::Absolution:
  {
    // Removes one monster of <= hero's level (except undead, bosses, other dungeon levels)
    auto monsterIt = glowingGuardian.pickMonsterForAbsolution(hero.getLevel(), allMonstersOnFloor);
    assert(monsterIt != end(allMonstersOnFloor));
    allMonstersOnFloor.erase(monsterIt);
    hero.changeHitPointsMax(4);
    if (!hero.receive(MiscItem::PrayerBead))
      assert(false);
    break;
  }
  case Boon::Cleansing:
    hero.reset(HeroDebuff::Poisoned);
    hero.reset(HeroDebuff::ManaBurned);
    hero.reduce(HeroDebuff::Weakened);
    hero.reduce(HeroDebuff::Corroded);
    hero.add(HeroStatus::ConsecratedStrike);
    if (!hero.receive(MiscItem::PrayerBead))
      assert(false);
    break;
  case Boon::Protection:
    hero.healHitPoints(hero.getHitPointsMax() * 35 / 100);
    hero.recoverManaPoints(hero.getManaPointsMax() * 35 / 100);
    if (!hero.receive(MiscItem::PrayerBead))
      assert(false);
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
    hero.reset(HeroDebuff::Weakened);
    hero.reset(HeroDebuff::Corroded);
    for (auto& monster : allMonstersOnFloor)
    {
      monster.changePhysicalResist(-20);
      monster.changeMagicResist(-20);
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
      monster.changeMagicResist(-10);
    break;

  case Boon::TaurogsBlade:
    receive(TaurogItem::Skullpicker);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsShield:
    receive(TaurogItem::Wereward);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsHelm:
    receive(TaurogItem::Gloat);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsArmour:
    receive(TaurogItem::Will);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::UnstoppableFury:
    hero.add(HeroStatus::DeathProtection);
    break;

  case Boon::Tribute:
    hero.spendGold(15);
    break;
  case Boon::TikkisEdge:
    hero.add(HeroStatus::Learning);
    hero.addGold(10);
    break;
  case Boon::Dodging:
    hero.addDodgeChancePercent(10, true);
    hero.addGold(10);
    break;
  case Boon::Poison:
    hero.add(HeroStatus::Poisonous, 1);
    break;
  case Boon::Reflexes:
    hero.lose(Potion::HealthPotion);
    receive(Potion::ReflexPotion);
    receive(Potion::QuicksilverPotion);
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
    return static_cast<int>(getPiety());
  const auto count = static_cast<int>(boonCount(boon));
  if (hero.has(HeroTrait::HolyWork))
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

void Faith::initialBoon(God god, Hero& hero, unsigned numRevealedTiles, Resources& resources)
{
  auto receiveFreeSpell = [&](Spell spell) {
    if (!hero.receiveFreeSpell(spell))
      resources().freeSpells.push_back(spell);
  };
  switch (god)
  {
  case God::BinlorIronshield:
    receiveFreeSpell(Spell::Pisorf);
    gainPiety(numRevealedTiles / 10u);
    break;
  case God::Dracul:
    gainPiety(2 * numMonstersKilled);
    break;
  case God::TheEarthmother:
    gainPiety(5);
    receiveFreeSpell(Spell::Imawal);
    break;
  case God::GlowingGuardian:
    gainPiety(5 * hero.getLevel());
    break;
  case God::JehoraJeheyu:
    gainPiety(jehora.initialPietyBonus(hero.getLevel(), hero.has(HeroStatus::Pessimist)));
    receiveFreeSpell(Spell::Weytwut);
    break;
  case God::MysteraAnnur:
    gainPiety(numSpellsCast);
    break;
  case God::Taurog:
    gainPiety(2 * numMonstersKilled);
    break;
  case God::TikkiTooki:
    receiveFreeSpell(Spell::Getindare);
    break;
  }
}

void Faith::punish(God god, Hero& hero, Monsters& allMonsters)
{
  if (hero.has(HeroTrait::HolyWork))
    return;
  switch (god)
  {
  case God::BinlorIronshield:
    hero.changePhysicalResistPercent(-50);
    hero.changeMagicalResistPercent(-50);
    break;
  case God::Dracul:
    hero.reduce(HeroStatus::LifeSteal);
    hero.changeHitPointsMax(-20);
    break;
  case God::TheEarthmother:
    hero.add(HeroDebuff::Corroded, allMonsters, 5);
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
      monster.changePhysicalResist(15);
      monster.changeMagicResist(15);
    }
    break;
  case God::Taurog:
    hero.changeDamageBonusPercent(-40);
    for (auto& monster : allMonsters)
      monster.changeMagicResist(10);
    break;
  case God::TikkiTooki:
    hero.reset(HeroStatus::DodgePermanent);
    hero.reset(HeroStatus::DodgeTemporary);
    hero.reset(HeroStatus::Poisonous);
    for (auto& monster : allMonsters)
      monster.applyTikkiTookiBoost();
    break;
  }
}

bool Faith::desecrate(God altar, Hero& hero, Monsters& allMonsters, bool hasAgnosticCollar)
{
  if (altarsForGoatperson)
    return false;
  if (!hasAgnosticCollar)
    punish(altar, hero, allMonsters);
  if (numDesecrated < 3)
    gainPiety((3 - numDesecrated) * 10);
  ++numDesecrated;
  indulgence += 3;
  return true;
}

PietyChange Faith::monsterKilled(const Monster& monster, unsigned heroLevel, bool monsterWasBurning)
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
        return monster.has(MonsterTrait::Undead) ? -5 : 2;
      case God::GlowingGuardian:
      {
        int award = 0;
        if (monster.has(MonsterTrait::Undead) && monster.grantsXP())
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
        if (monster.has(MonsterTrait::MagicalAttack))
          return -5;
        break;
      case God::Taurog:
        return monster.has(MonsterTrait::MagicalAttack) ? 8 : 4;
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

PietyChange Faith::spellCast(Spell spell, unsigned manaCost)
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
  {
    const auto numManaPointsSpentBefore = numManaPointsSpent;
    numManaPointsSpent += manaCost;
    if (preparationPenaltyApplies())
    {
      // 2 piety are awarded for 5 mana points spent.
      int reward = numManaPointsSpent / 5 * 2;
      numManaPointsSpent %= 5;
      // However, 1 piety of these is already awarded after 3 MP.
      if (numManaPointsSpentBefore >= 3)
        reward -= 1;
      if (numManaPointsSpent >= 3)
        reward += 1;
      return reward;
    }
    else if (numManaPointsSpent >= 2)
    {
      const int reward = numManaPointsSpent / 2;
      numManaPointsSpent %= 2;
      return reward;
    }
    break;
  }
  case God::Taurog:
    return -2;
  case God::TikkiTooki:
    if (spell == Spell::Weytwut || spell == Spell::Wonafyt)
      return 1;
    break;
  }
  return {};
}

PietyChange Faith::imawalCreateWall(unsigned manaCost)
{
  if (followedDeity == God::BinlorIronshield)
    return -5;
  if (followedDeity == God::TheEarthmother)
    return {};
  return spellCast(Spell::Imawal, manaCost);
}

PietyChange Faith::imawalPetrifyPlant(unsigned manaCost)
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
        ++numConsecutiveLevelUpsWithGlowingGuardian;
        const auto piety = preparationPenaltyApplies() ? 2u * (numConsecutiveLevelUpsWithGlowingGuardian - 1u)
                                                       : 3u * numConsecutiveLevelUpsWithGlowingGuardian;
        return static_cast<int>(piety);
      }
      return {};
    }();
  }
  return result;
}

PietyChange Faith::drankPotion(Potion potion)
{
  PietyChange result;
  if (potion == Potion::HealthPotion || potion == Potion::ManaPotion)
  {
    if (pact == Pact::AlchemistsPact)
      result += *pact;
    if (followedDeity)
    {
      result += [&, deity = *followedDeity]() -> PietyChange {
        if (deity == God::GlowingGuardian)
          return -10;
        else if (potion == Potion::HealthPotion && deity == God::Dracul)
          return -5;
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

PietyChange Faith::bloodPoolConsumed(unsigned numBloodTithe)
{
  if (followedDeity == God::Dracul && numBloodTithe > 0)
    return static_cast<int>(numBloodTithe);
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

PietyChange Faith::manaPointsBurned(unsigned pointsLost)
{
  if (followedDeity == God::MysteraAnnur)
    return -static_cast<int>(pointsLost);
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
        if (const auto item = std::get_if<Item>(&itemOrSpell); item && std::holds_alternative<Potion>(*item))
          return 5;
        return 2;
      case God::JehoraJeheyu:
        return JehoraTriggered();
      case God::Taurog:
        if (std::holds_alternative<Spell>(itemOrSpell))
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
    auto& monsterHistory = history[monster.getID()];
    if (preparationPenaltyApplies() || monsterHistory.hitHero)
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
  if (hero.has(HeroTrait::HolyWork))
    return;
  hero.changeDamageBonusPercent(-10);
  for (auto& monster : allMonsters)
    monster.changeMagicResist(10);
}
