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

  case Boon::Humility:
    hero.modifyLevelBy(-1);
    break;

  case Boon::Flames:
    hero.changeDamageBonusPercent(-50);

  default:
    break;
  }
  return true;
}

int Faith::getCosts(Boon boon, const Hero& hero) const
{
  const int baseCosts = [boon, count = boonCount(boon)] {
    switch (boon)
    {
    case Boon::StoneSoup:
      return 35;
    case Boon::StoneSkin:
      return 15;
    case Boon::StoneForm:
      return 25;
    case Boon::StoneFist:
      return 40;
    case Boon::StoneHeart:
      return 10;

    case Boon::BloodCurse:
      return -20;
    case Boon::BloodTithe:
      return 10 + count * 15;
    case Boon::BloodHunger:
      return 20 + count * 25;
    case Boon::BloodShield:
      return 40;
    case Boon::BloodSwell:
      return 20 + count * 10;

    case Boon::Humility:
      return 15;
    case Boon::Petition:
      return 45;
    case Boon::Flames:
      return 20;
    }
  }();
  if (hero.hasTrait(HeroTrait::HolyWork))
    return baseCosts * 8 / 10;
  return baseCosts;
}

int Faith::isAvailable(Boon boon, const Hero& hero) const
{
  // TODO: Check number of available walls / petrified enemies for Binlor's boons
  return deity(boon) == followedDeity && (allowRepeatedUse(boon) || !boonCount(boon)) &&
         (boon != Boon::BloodCurse || hero.getLevel() < 10) && (boon != Boon::Humility || hero.getLevel() > 1);
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
    // TODO: Random piety based on level
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
    return isSmall(item) ? 2 : 5;
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
