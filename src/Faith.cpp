#include "Faith.hpp"

#include "Hero.hpp"
#include "Spells.hpp"

#include <algorithm>
#include <cassert>

bool Faith::followDeity(God god, Hero& hero)
{
  if (hero.hasTrait(HeroTrait::Damned) || hero.hasTrait(HeroTrait::Scapegoat))
    return false;
  if (followedDeity.has_value() && *followedDeity != god)
  {
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

bool Faith::hasBoon(Boon boon) const
{
  return std::find(begin(boons), end(boons), boon) != end(boons);
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

void Faith::applyRandomPunishment(Hero& hero)
{
  jehora.applyRandomPunishment(hero);
}

void Faith::updatePiety(std::optional<int> deltaPoints, Hero& hero)
{
  if (!deltaPoints)
    return;
  if (*deltaPoints > 0)
    gainPiety(*deltaPoints);
  else if (*deltaPoints < 0)
    losePiety(-*deltaPoints, hero);
  else
    applyRandomPunishment(hero);
}

// bool request(Boon boon);
// int getCosts(Boon boon) const;
// int isAvailable(Boon boon) const;

void Faith::initialBoon(God god, Hero& hero)
{
  switch (god)
  {
  case God::BinlorIronshield:
    // TODO: hero.receiveFreeSpell(Spell::Pisorf);
    // TODO: gainPiety(numRevealedTiles / 10);
    break;
  case God::Dracul:
    gainPiety(2 * numMonstersKilled);
    break;
  case God::TheEarthmother:
    gainPiety(5);
    // TODO: hero.receiveFreeSpell(Spell::Imawal);
    break;
  case God::GlowingGuardian:
    gainPiety(5 * hero.getLevel());
    break;
  case God::JehoraJeheyu:
    // TODO: Random piety based on level
    // TODO: hero.receiveFreeSpell(Spell::Weytwut);
    break;
  case God::MysteraAnnur:
    gainPiety(numSpellsCast);
    break;
  case God::Taurog:
    gainPiety(2 * numMonstersKilled);
    break;
  case God::TikkiTooki:
    // TODO: hero.receiveFreeSpell(Spell::Getindare);
    break;
  }
}

void Faith::punish(God god, Hero& hero)
{
  switch (god)
  {
  case God::BinlorIronshield:
    hero.changePhysicalResistPercent(-50);
    hero.changeMagicalResistPercent(-50);
    break;
  case God::Dracul:
    hero.removeStatus(HeroStatus::LifeSteal, false);
    hero.setHitPointsMax(std::max(1, hero.getHitPointsMax() - 20));
    break;
  case God::TheEarthmother:
    hero.addStatus(HeroStatus::Corrosion, 5);
    break;
  case God::GlowingGuardian:
    // TODO: hero.loseAllItems();
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

std::optional<int> Faith::monsterKilled(const Monster& monster, int heroLevel, bool monsterWasBurning)
{
  ++numMonstersKilled;
  if (!followedDeity)
    return std::nullopt;
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
      return jehora();
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
  return std::nullopt;
}

std::optional<int> Faith::monsterPoisoned(const Monster& monster)
{
  if (!followedDeity)
    return std::nullopt;
  if (*followedDeity == God::TikkiTooki)
  {
    auto& events = pietyEvents[monster.getID()];
    if (!events.becamePoisoned)
    {
      events.becamePoisoned = true;
      return 1;
    }
  }
  else if (*followedDeity == God::GlowingGuardian)
    return -10;
  return std::nullopt;
}

std::optional<int> Faith::spellCast(Spell spell, int manaCost)
{
  ++numSpellsCast;
  if (!followedDeity)
    return std::nullopt;
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
      return jehora();
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
  return std::nullopt;
}

std::optional<int> Faith::imawalCreateWall(int manaCost)
{
  if (followedDeity == God::BinlorIronshield)
    return -5;
  if (followedDeity == God::TheEarthmother)
    return std::nullopt;
  return spellCast(Spell::Imawal, manaCost);
}

std::optional<int> Faith::imawalPetrifyPlant(int manaCost)
{
  if (followedDeity == God::TheEarthmother)
    return 5;
  return spellCast(Spell::Imawal, manaCost);
}

std::optional<int> Faith::levelGained()
{
  if (followedDeity == God::BinlorIronshield)
    return -10;
  if (followedDeity == God::GlowingGuardian)
  {
    // Preparation penalty for Glowing Guardian: reduce level-up bonus to 2 * (N - 1)
    ++numConsecutiveLevelUpsWithGlowingGuardian;
    return 3 * numConsecutiveLevelUpsWithGlowingGuardian;
  }
  return std::nullopt;
}

std::optional<int> Faith::potionConsumed(/* Potion potion */) {
  return std::nullopt;
}

std::optional<int> Faith::lifeStolen(const Monster& monster) {
    return std::nullopt;
}

std::optional<int> Faith::becamePoisoned() {
    return std::nullopt;
}

std::optional<int> Faith::manaBurned() {
    return std::nullopt;
}

//std::optional<int> Faith::converted(Item item) {}

std::optional<int> Faith::converted(/* Potion potion */) {
    return std::nullopt;
}

std::optional<int> Faith::converted(Spell spell) {
    return std::nullopt;
}

std::optional<int> Faith::wallDestroyed() {
    return std::nullopt;
}

std::optional<int> Faith::bloodPoolConsumed() {
    return std::nullopt;
}

std::optional<int> Faith::plantDestroyed() {
    return std::nullopt;
}

std::optional<int> Faith::receivedHit(const Monster& monster) {
    return std::nullopt;
}

std::optional<int> Faith::dodgedAttack() {
    return std::nullopt;
}

std::optional<int> Faith::deathProtectionTriggered() {
    return std::nullopt;
}

/*
if (!followedDeity)
  return;
switch (*followedDeity)
{
case God::BinlorIronshield:
  break;
case God::Dracul:
  break;
case God::TheEarthmother:
  break;
case God::GlowingGuardian:
  break;
case God::JehoraJeheyu:
  break;
case God::MysteraAnnur:
  break;
case God::Taurog:
  break;
case God::TikkiTooki:
  break;
}
*/

Faith::Jehora::Jehora()
  : generator(std::random_device{}())
  , happiness(14)
  , thresholdPoison(0)
  , thresholdManaBurn(1)
  , thresholdHealthLoss(1)
  , thresholdWeakened(0)
  , thresholdCorrosion(0)
  , thresholdCursed(0)
{
}

int Faith::Jehora::initialPietyBonus(int heroLevel)
{
  std::uniform_int_distribution<> initialBonus(70, 100);
  return initialBonus(generator) * heroLevel * heroLevel / 100;
}

int Faith::Jehora::operator()()
{
  // Randomly award 2-4 XP or apply random punishment
  std::uniform_int_distribution<> happinessRoll(1, 15);
  std::uniform_int_distribution<> pietyRoll(2, 4);
  if (happinessRoll(generator) <= happiness)
  {
    happiness = std::max(happiness - 1, 5);
    return pietyRoll(generator);
  }
  // a punishment is to be applied
  happiness = std::min(happiness + 1, 14);
  return 0;
}

void Faith::Jehora::applyRandomPunishment(Hero& hero)
{
  if (hero.hasBoon(Boon::Petition))
    return;
  std::uniform_int_distribution<> punishmentRoll(0, 10);
  int rerolls = 0;
  while (true)
  {
    const int roll = punishmentRoll(generator);
    const int category = roll / 3;
    switch (category)
    {
    case 0:
    {
      switch (roll)
      {
      case 0:
        if (rerolls >= thresholdPoison)
        {
          hero.addStatus(HeroStatus::Poisoned);
          ++thresholdPoison;
          return;
        }
        break;
      case 1:
        if (rerolls >= thresholdManaBurn)
        {
          hero.addStatus(HeroStatus::ManaBurned);
          ++thresholdManaBurn;
          return;
        }
        break;
      case 2:
        if (rerolls >= thresholdHealthLoss)
        {
          hero.loseHitPointsOutsideOfFight(hero.getHitPoints() - 1);
          ++thresholdHealthLoss;
          return;
        }
      }
    }
    case 1:
      if (rerolls >= thresholdWeakened)
      {
        hero.addStatus(HeroStatus::Weakened);
        ++thresholdWeakened;
        return;
      }
      break;
    case 2:
      if (rerolls >= thresholdCorrosion)
      {
        hero.addStatus(HeroStatus::Corrosion);
        ++thresholdCorrosion;
      }
      break;
    case 3:
      if (rerolls >= thresholdCursed)
      {
        hero.addStatus(HeroStatus::Cursed);
        ++thresholdCursed;
        return;
      }
      break;
    }
    ++rerolls;
  }
}
