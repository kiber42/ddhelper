#include "engine/Monster.hpp"

#include "engine/MonsterTypes.hpp"

#include <algorithm>
#include <utility>

int Monster::lastId = 0;

namespace
{
  std::string makeMonsterName(MonsterType type, int level)
  {
    using namespace std::string_literals;
    return toString(type) + " level "s + std::to_string(level);
  }
} // namespace

Monster::Monster(MonsterType type, int level, int dungeonMultiplier)
  : name(makeMonsterName(type, level))
  , id(++lastId)
  , stats(type, level, dungeonMultiplier)
  , defence(type)
  , traits(type)
{
}

Monster::Monster(std::string name, MonsterStats stats, Defence damage, MonsterTraits traits)
  : name(std::move(name))
  , id(++lastId)
  , stats(std::move(stats))
  , defence(std::move(damage))
  , status{}
  , traits(std::move(traits))
{
}

Monster::Monster(int level, int hp, int damage)
  : Monster("Monster level " + std::to_string(level), {level, hp, damage, 0}, {}, {})
{
}

const char* Monster::getName() const
{
  return name.c_str();
}

int Monster::getID() const
{
  return id;
}

int Monster::getLevel() const
{
  return stats.getLevel();
}

bool Monster::isDefeated() const
{
  return stats.isDefeated();
}

int Monster::getHitPoints() const
{
  return stats.getHitPoints();
}

int Monster::getHitPointsMax() const
{
  return stats.getHitPointsMax();
}

int Monster::getDamage() const
{
  const int standardDamage = stats.getDamage();
  const int berserkLimit = traits.getBerserkPercent() * getHitPointsMax();
  if (berserkLimit > 0 && getHitPoints() <= berserkLimit)
    return standardDamage * 3 / 2;
  return standardDamage;
}

int Monster::getPhysicalResistPercent() const
{
  return defence.getPhysicalResistPercent();
}

int Monster::getMagicalResistPercent() const
{
  return defence.getMagicalResistPercent();
}

int Monster::predictDamageTaken(int attackerDamageOutput, DamageType damageType) const
{
  return defence.predictDamageTaken(attackerDamageOutput, damageType, status.getBurnStackSize());
}

void Monster::takeDamage(int attackerDamageOutput, DamageType damageType)
{
  stats.loseHitPoints(predictDamageTaken(attackerDamageOutput, damageType));
  status.setSlowed(false);
  status.setBurn(0);
}

void Monster::takeFireballDamage(int casterLevel, int damageMultiplier)
{
  const int damagePoints = casterLevel * damageMultiplier;
  stats.loseHitPoints(predictDamageTaken(damagePoints, DamageType::Magical));
  status.setSlowed(false);
  burn(casterLevel * 2);
}

void Monster::takeBurningStrikeDamage(int attackerDamageOutput, int casterLevel, DamageType damageType)
{
  stats.loseHitPoints(predictDamageTaken(attackerDamageOutput, damageType));
  status.setSlowed(false);
  burn(casterLevel * 2);
}

void Monster::takeManaShieldDamage(int casterLevel)
{
  // Mana Shield damage against magical resistance is rounded down (usually resisted damage is rounded down)
  stats.loseHitPoints(casterLevel * (100 - getMagicalResistPercent()) / 100);
}

void Monster::receiveCrushingBlow()
{
  int delta = stats.getHitPoints() - stats.getHitPointsMax() * 3 / 4;
  if (delta > 0)
    stats.loseHitPoints(delta);
  status.setSlowed(false);
  status.setBurn(0);
}

void Monster::recover(int nSquares)
{
  if (isDefeated())
    return;
  int recoverPoints = nSquares * (getLevel() - static_cast<int>(status.isBurning()));
  if (traits.hasFastRegen())
    recoverPoints *= 2;
  if (status.isPoisoned())
  {
    int poison = status.getPoisonAmount();
    if (poison >= recoverPoints)
    {
      status.setPoison(poison - recoverPoints);
      recoverPoints = 0;
    }
    else
    {
      recoverPoints -= poison;
      status.setPoison(0);
    }
  }
  if (recoverPoints > 0)
    stats.healHitPoints(recoverPoints, false);
}

void Monster::burn(int nMaxStacks)
{
  status.setBurn(std::min(status.getBurnStackSize() + 1, nMaxStacks));
}

void Monster::burnMax(int nMaxStacks)
{
  status.setBurn(nMaxStacks);
}

void Monster::burnDown()
{
  if (status.getBurnStackSize() > 0)
  {
    const int resist = status.getBurnStackSize() * getMagicalResistPercent();
    stats.loseHitPoints(status.getBurnStackSize() - resist);
    status.setSlowed(false);
    status.setBurn(0);
  }
}

bool Monster::poison(int addedPoisonAmount)
{
  if (isUndead())
    return false;
  status.setPoison(status.getPoisonAmount() + addedPoisonAmount);
  return true;
}

void Monster::slow()
{
  status.setSlowed(true);
}

void Monster::erodeResitances()
{
  defence.setPhysicalResistPercent(defence.getPhysicalResistPercent() - 3);
  defence.setMagicalResistPercent(defence.getMagicalResistPercent() - 3);
}

void Monster::petrify()
{
  die();
}

void Monster::die()
{
  status.setBurn(false);
  status.setSlowed(false);
  stats.setDeathProtection(0);
  stats.loseHitPoints(stats.getHitPoints());
}

void Monster::corrode(int amount)
{
  status.setCorroded(getCorroded() + amount);
  defence.setCorrosion(getCorroded());
}

void Monster::zot()
{
  if (!status.isZotted())
  {
    status.setZotted();
    stats.setHitPointsMax(stats.getHitPointsMax() / 2);
  }
}

void Monster::makeWickedSick()
{
  const int level = getLevel();
  if (level < 10 && !status.isWickedSick())
  {
    const auto type = stats.getType();
    const int newLevel = level + 1;
    const bool hasStandardName = name == makeMonsterName(type, level);
    status.setWickedSick();
    stats = MonsterStats(stats.getType(), newLevel, stats.getDungeonMultiplier());
    if (hasStandardName)
      name = makeMonsterName(type, newLevel);
  }
}

bool Monster::isBurning() const
{
  return status.isBurning();
}

bool Monster::isPoisoned() const
{
  return status.isPoisoned();
}

bool Monster::isSlowed() const
{
  return status.isSlowed();
}

bool Monster::isZotted() const
{
  return status.isZotted();
}

bool Monster::isWickedSick() const
{
  return status.isWickedSick();
}

int Monster::getBurnStackSize() const
{
  return status.getBurnStackSize();
}

int Monster::getPoisonAmount() const
{
  return status.getPoisonAmount();
}

int Monster::getDeathProtection() const
{
  return stats.getDeathProtection();
}

int Monster::getCorroded() const
{
  return status.getCorroded();
}

bool Monster::doesMagicalDamage() const
{
  return traits.doesMagicalDamage();
}

DamageType Monster::damageType() const
{
  return doesMagicalDamage() ? DamageType::Magical : DamageType::Physical;
}

bool Monster::doesRetaliate() const
{
  return traits.doesRetaliate();
}

bool Monster::isPoisonous() const
{
  return traits.isPoisonous();
}

bool Monster::hasManaBurn() const
{
  return traits.hasManaBurn();
}

bool Monster::bearsCurse() const
{
  return traits.bearsCurse();
}

bool Monster::isCorrosive() const
{
  return traits.isCorrosive();
}

bool Monster::isWeakening() const
{
  return traits.isWeakening();
}

bool Monster::hasFirstStrike() const
{
  return traits.hasFirstStrike();
}

int Monster::getDeathGazePercent() const
{
  return traits.getDeathGazePercent();
}

int Monster::getLifeStealPercent() const
{
  return traits.getLifeStealPercent();
}

bool Monster::isUndead() const
{
  return traits.isUndead();
}

bool Monster::isBloodless() const
{
  return traits.isBloodless();
}

bool Monster::isCowardly() const
{
  return traits.isCowardly();
}

void Monster::addPhysicalResist(int additionalResistPercent)
{
  defence.setPhysicalResistPercent(defence.getPhysicalResistPercent() + additionalResistPercent);
}

void Monster::addMagicResist(int additionalResistPercent)
{
  defence.setMagicalResistPercent(defence.getMagicalResistPercent() + additionalResistPercent);
}

void Monster::makeFast()
{
  traits.makeFast();
}

void Monster::makeWeakening()
{
  traits.makeWeakening();
}

using namespace std::string_literals;

std::vector<std::string> describe(const Monster& monster)
{
  if (monster.isDefeated())
    return {monster.getName() + " defeated."s};

  std::vector description{std::string{monster.getName()},
                          std::to_string(monster.getHitPoints()) + "/" + std::to_string(monster.getHitPointsMax()) +
                              " HP",
                          std::to_string(monster.getDamage()) + " damage"};
  if (monster.hasFirstStrike() && !monster.isSlowed())
    description.emplace_back("First Strike");
  if (monster.doesMagicalDamage())
    description.emplace_back("Magical Attack");
  if (monster.doesRetaliate() && !monster.isSlowed())
    description.emplace_back("Retaliate: Fireball");
  if (monster.getPhysicalResistPercent() > 0)
    description.emplace_back("Physical resist "s + std::to_string(monster.getPhysicalResistPercent()) + "%");
  if (monster.getMagicalResistPercent() > 0)
    description.emplace_back("Magical resist "s + std::to_string(monster.getMagicalResistPercent()) + "%");
  if (monster.isPoisonous())
    description.emplace_back("Poisonous");
  if (monster.hasManaBurn())
    description.emplace_back("Mana Burn");
  if (monster.bearsCurse())
    description.emplace_back("Curse bearer");
  if (monster.isCorrosive())
    description.emplace_back("Corrosive");
  if (monster.isWeakening())
    description.emplace_back("Weakening Blow");
  if (monster.getDeathGazePercent() > 0)
    description.emplace_back("Death Gaze "s + std::to_string(monster.getDeathGazePercent()) + "%");
  if (monster.getDeathProtection() > 0)
    description.emplace_back("Death protection (x"s + std::to_string(monster.getDeathProtection()) + "%");
  if (monster.getLifeStealPercent() > 0)
    description.emplace_back("Life Steal "s + std::to_string(monster.getLifeStealPercent()) + "%");
  if (monster.isBurning())
    description.emplace_back("Burning (burn stack size "s + std::to_string(monster.getBurnStackSize()) + ")");
  if (monster.isPoisoned())
    description.emplace_back("Poisoned (amount: "s + std::to_string(monster.getPoisonAmount()) + ")");
  if (monster.isSlowed())
    description.emplace_back("Slowed");
  if (monster.isZotted())
    description.emplace_back("Zotted");
  if (monster.isWickedSick())
    description.emplace_back("Wicked Sick");
  if (monster.getCorroded() > 0)
    description.emplace_back("Corroded (x"s + std::to_string(monster.getCorroded()) + ")");
  if (monster.isUndead())
    description.emplace_back("Undead");
  if (monster.isBloodless())
    description.emplace_back("Bloodless");
  if (!monster.grantsXP())
    description.emplace_back("No Experience");

  return description;
}
