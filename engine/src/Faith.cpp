#include "Faith.hpp"

#include "Hero.hpp"
#include "Items.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>

PietyChange::PietyChange(int deltaPoints)
  : value(deltaPoints)
{
}

PietyChange::PietyChange(JehoraTriggered)
  : value(std::nullopt)
{
}

int PietyChange::operator()() const
{
  return value.value_or(0);
}

bool PietyChange::randomJehoraEvent() const
{
  return !value.has_value();
}

PietyChange& PietyChange::operator+=(const PietyChange& other)
{
  if (!value || !other.value)
    value.reset();
  else
    value = *value + *other.value;
  return *this;
}

bool Faith::followDeity(God god, Hero& hero)
{
  if (hero.hasTrait(HeroTrait::Damned) || hero.hasTrait(HeroTrait::Scapegoat))
    return false;
  if (followedDeity.has_value())
  {
    if (followedDeity == god)
      return false;
    if (hero.hasTrait(HeroTrait::HolyWork))
      return false;
    if (piety < 50)
      return false;
    piety /= 2;
    followedDeity = god;
    numConsecutiveLevelUpsWithGlowingGuardian = 0;
  }
  else
  {
    followedDeity = god;
    initialBoon(god, hero);
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

int Faith::boonCount(Boon boon) const
{
  if (!allowRepeatedUse(boon))
    return std::find(begin(boons), end(boons), boon) != end(boons) ? 1 : 0;
  return std::count(begin(boons), end(boons), boon);
}

void Faith::gainPiety(int pointsGained)
{
  if (pointsGained > 0)
    piety = std::min(piety + pointsGained, getMaxPiety());
}

void Faith::losePiety(int pointsLost, Hero& hero)
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
    punish(*followedDeity, hero);
  }
}

void Faith::applyRandomJehoraEvent(Hero& hero)
{
  const int result = jehora();
  if (result > 0)
    gainPiety(result);
  else
    jehora.applyRandomPunishment(hero);
}

void Faith::apply(PietyChange change, Hero& hero)
{
  const int value = change();
  if (value > 0)
    gainPiety(value);
  else if (value < 0)
    losePiety(-value, hero);
  else if (change.randomJehoraEvent())
    applyRandomJehoraEvent(hero);
}

bool Faith::request(Boon boon, Hero& hero)
{
  if (!isAvailable(boon, hero))
    return false;
  const int costs = getCosts(boon, hero);
  if (costs > piety)
    return false;
  piety -= costs;
  boons.push_back(boon);
  // Apply immediate effects.
  switch (boon)
  {
  case Boon::StoneSoup:
    hero.receiveFreeSpell(Spell::Endiswal);
    break;
  case Boon::StoneSkin:
    // TODO: Destroys 3 nearby walls
    hero.addStatus(HeroStatus::StoneSkin, 3);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 3);
    break;
  case Boon::StoneForm:
    // TODO: Destroys 10 nearby walls
    // TODO: Might is added whenever walls are destroyed
    hero.addStatus(HeroStatus::Might);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 5);
    break;
  case Boon::StoneFist:
    // TODO: Destroys 20 nearby walls
    hero.addStatus(HeroStatus::Knockback, 50);
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 5);
    break;
  case Boon::StoneHeart:
    // TODO: Destroys 15 nearby walls
    // TODO: Lower resistances of all enemies on current dungeon level by 5%
    hero.setMagicalResistPercent(hero.getMagicalResistPercent() + 3);
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
    hero.addStatus(HeroStatus::Cursed);
    break;

  case Boon::Plantation:
    // TODO: Grow plants on each visible bloodstain, receive 5 piety per new plant
    gainPiety(10);
    break;
  case Boon::Clearance:
    // TODO: Remove up to 10 plants, restore 1 MP per removed plant
    hero.recoverManaPoints(5);
    break;
  case Boon::Greenblood:
    // TODO: Every monster receives +1 Corrosion
    hero.changeBaseDamage(1); // TODO: Remove
    // TODO: Spawns 3 random plants
    hero.removeStatus(HeroStatus::Cursed, false);
    break;
  case Boon::Entanglement:
    // TODO: Every monster is slowed
    // TODO: Spawns 5 random plants
    break;
  case Boon::VineForm:
    hero.changeHitPointsMax(+4);
    hero.addStatus(HeroStatus::DamageReduction);
    // TODO: Spawns 2 random plants
    break;

  case Boon::Humility:
    hero.modifyLevelBy(-1);
    break;
  case Boon::Absolution:
    // TODO: Precondition: Removes one monster of <= hero's level (except undead, bosses, other dungeon levels)
    hero.changeHitPointsMax(4);
    hero.receive(Item::PrayerBead);
    break;
  case Boon::Cleansing:
    hero.removeStatus(HeroStatus::Poisoned, true);
    hero.removeStatus(HeroStatus::ManaBurned, true);
    hero.removeStatus(HeroStatus::Weakened, false);
    hero.removeStatus(HeroStatus::Corrosion, false);
    hero.addStatus(HeroStatus::ConsecratedStrike);
    hero.receive(Item::PrayerBead);
    break;
  case Boon::Protection:
    hero.healHitPoints(hero.getHitPointsMax() * 35 / 100);
    hero.recoverManaPoints(hero.getManaPointsMax() * 35 / 100);
    hero.receive(Item::PrayerBead);
    break;
  case Boon::Enlightenment:
  {
    hero.receiveEnlightenment();
    break;
  }

  case Boon::LastChance:
    if (jehora.lastChanceSuccessful(costs))
      hero.fullHealthAndMana();
    break;
  case Boon::BoostHealth:
    hero.lose(Item::HealthPotion);
    hero.changeHitPointsMax(+20);
    break;
  case Boon::BoostMana:
    hero.lose(Item::ManaPotion);
    hero.changeManaPointsMax(+3);
    break;
  case Boon::ChaosAvatar:
    hero.gainLevel();
    hero.fullHealthAndMana(); // for Goatperson
    hero.addConversionPoints(100);
    hero.removeStatus(HeroStatus::Weakened, true);
    hero.removeStatus(HeroStatus::Corrosion, true);
    // TODO: -20% physical and magic resist for all monsters on current level
    break;

  case Boon::Magic:
    hero.changeManaPointsMax(+1);
    break;
  case Boon::Flames:
    hero.changeDamageBonusPercent(-50);
    break;
  case Boon::Weakening:
    hero.changeMagicalResistPercentMax(-10);
    // TODO: -10% magic resistance to all monsters on current dungeon floor
    break;

  case Boon::TaurogsBlade:
    hero.receive(Item::Skullpicker);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsShield:
    hero.receive(Item::Wereward);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsHelm:
    hero.receive(Item::Gloat);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::TaurogsArmour:
    hero.receive(Item::Will);
    hero.changeDamageBonusPercent(+5);
    hero.changeManaPointsMax(-1);
    break;
  case Boon::UnstoppableFury:
    hero.addStatus(HeroStatus::DeathProtection);
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

int Faith::isAvailable(Boon boon, const Hero& hero) const
{
  // TODO: Check number of available walls / petrified enemies for Binlor's boons
  return deity(boon) == followedDeity && (allowRepeatedUse(boon) || !boonCount(boon)) &&
         (boon != Boon::BloodCurse || hero.getLevel() < 10) && (boon != Boon::Humility || hero.getLevel() > 1) &&
         (boon != Boon::BoostHealth || hero.has(Item::HealthPotion)) &&
         (boon != Boon::BoostMana || hero.has(Item::ManaPotion)) &&
         (boon != Boon::UnstoppableFury ||
          (!hero.hasStatus(HeroStatus::DeathProtection) && hero.has(Item::Skullpicker) && hero.has(Item::Wereward) &&
           hero.has(Item::Gloat) && hero.has(Item::Will)));
}

void Faith::initialBoon(God god, Hero& hero)
{
  switch (god)
  {
  case God::BinlorIronshield:
    hero.receiveFreeSpell(Spell::Pisorf);
    // TODO: gainPiety(numRevealedTiles / 10);
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
    gainPiety(jehora.initialPietyBonus(hero.getLevel()));
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

void Faith::punish(God god, Hero& hero)
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
    hero.removeStatus(HeroStatus::LifeSteal, false);
    hero.changeHitPointsMax(-20);
    break;
  case God::TheEarthmother:
    hero.addStatus(HeroStatus::Corrosion, 5);
    break;
  case God::GlowingGuardian:
    hero.loseAllItems();
    break;
  case God::JehoraJeheyu:
    // TODO: 50% chance
    hero.loseHitPointsOutsideOfFight(hero.getHitPoints() - 1);
    hero.loseManaPoints(hero.getManaPoints());
    hero.setHitPointsMax(hero.getHitPointsMax() * 2 / 3);
    hero.setManaPointsMax(hero.getManaPointsMax() * 2 / 3);
    hero.changeDamageBonusPercent(-33);
    break;
  case God::MysteraAnnur:
    // TODO: 15% physical and magical resistance for all monsters on all dungeon floors
    break;
  case God::Taurog:
    hero.changeDamageBonusPercent(-40);
    // TODO: 10% magical resistance for all monsters
    break;
  case God::TikkiTooki:
    hero.removeStatus(HeroStatus::DodgePermanent, true);
    hero.removeStatus(HeroStatus::DodgeTemporary, true);
    // TODO: hero.removeStatus(HeroStatus::PoisonStrike, true);
    // TODO: All monsters on all dungeon floors gain first strike and weakening blow
    break;
  }
}

void Faith::desecrate(God altar, Hero& hero)
{
  punish(altar, hero);
  if (numDesecrated < 3)
    gainPiety((3 - numDesecrated) * 10);
  ++numDesecrated;
  indulgence += 3;
}

PietyChange Faith::monsterKilled(const Monster& monster, int heroLevel, bool monsterWasBurning)
{
  ++numMonstersKilled;
  if (!followedDeity)
    return {};
  switch (*followedDeity)
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
    if (spell == Spell::Endiswal)
      return 5;
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
  if (followedDeity == God::BinlorIronshield)
    return -10;
  if (followedDeity == God::GlowingGuardian)
  {
    // TODO: Preparation penalty for Glowing Guardian: reduce level-up bonus to 2 * (N - 1)
    ++numConsecutiveLevelUpsWithGlowingGuardian;
    return 3 * numConsecutiveLevelUpsWithGlowingGuardian;
  }
  return {};
}

PietyChange Faith::itemUsed(Item item)
{
  if (item == Item::HealthPotion)
  {
    if (followedDeity == God::Dracul)
      return -5;
    if (followedDeity == God::GlowingGuardian)
      return -10;
  }
  else if (item == Item::ManaPotion)
  {
    if (followedDeity == God::GlowingGuardian)
      return -10;
  }
  return {};
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
  if (followedDeity == God::Dracul)
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

PietyChange Faith::converted(Item item)
{
  if (!followedDeity)
    return {};
  switch (*followedDeity)
  {
  case God::Dracul:
    if (item == Item::HealthPotion)
      return 5;
    return {};
  case God::GlowingGuardian:
    return isPotion(item) || !isSmall(item) ? 5 : 2;
  case God::JehoraJeheyu:
    return JehoraTriggered();
  case God::Taurog:
    // TODO: Converting any of Taurog's items: -10 (except potentially in triple quests)
  default:
    return {};
  }
}

PietyChange Faith::converted(Spell spell)
{
  if (!followedDeity)
    return {};
  switch (*followedDeity)
  {
  case God::Dracul:
    if (spell == Spell::Cydstepp || spell == Spell::Halpmeh)
      return 10;
    return {};
  case God::GlowingGuardian:
    return spell == Spell::Apheelsik || spell == Spell::Bludtupowa ? 10 : 5;
  case God::JehoraJeheyu:
    return JehoraTriggered();
  case God::Taurog:
    return 10;
  default:
    return {};
  }
}

PietyChange Faith::plantDestroyed()
{
  if (followedDeity == God::TheEarthmother)
    return -15;
  return {};
}

PietyChange Faith::receivedHit(const Monster& monster)
{
  if (followedDeity == God::TikkiTooki)
  {
    // TODO: If Tikki Tooki was equipped, any hit results in a penalty
    auto& monsterHistory = history[monster.getID()];
    if (monsterHistory.hitHero)
      return -3;
    monsterHistory.hitHero = true;
  }
  return {};
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

void Faith::convertedTaurogItem(Hero& hero)
{
  hero.changeDamageBonusPercent(-10);
  // TODO: 10% magical resistance for all monsters
}
