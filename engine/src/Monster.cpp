#include "engine/Monster.hpp"

#include "engine/Clamp.hpp"
#include "engine/MonsterTypes.hpp"

#include <algorithm>
#include <utility>

int Monster::lastId = 0;

namespace
{
  std::string makeMonsterName(MonsterType type, Level level)
  {
    using namespace std::string_literals;
    return toString(type) + " level "s + std::to_string(level.get());
  }
} // namespace

Monster::Monster(MonsterType type, uint8_t level, uint8_t dungeonMultiplier)
  : name(makeMonsterName(type, Level{level}))
  , id(++lastId)
  , stats(type, Level{level}, DungeonMultiplier{dungeonMultiplier})
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

Monster::Monster(uint8_t level, uint16_t hp, uint16_t damage)
  : Monster("Monster level " + std::to_string(level), {Level{level}, HitPoints{hp}, DamagePoints{damage}, DeathProtection{0}}, {}, {})
{
}

const std::string& Monster::getName() const
{
  return name;
}

int Monster::getID() const
{
  return id;
}

unsigned Monster::getLevel() const
{
  return stats.getLevel().get();
}

bool Monster::isDefeated() const
{
  return stats.isDefeated();
}

unsigned Monster::getHitPoints() const
{
  return stats.getHitPoints().get();
}

unsigned Monster::getHitPointsMax() const
{
  return stats.getHitPointsMax().get();
}

unsigned Monster::getDamage() const
{
  const auto standardDamage = stats.getDamage().get();
  const auto berserkLimit = traits.getBerserkPercent() * getHitPointsMax();
  if (berserkLimit > 0 && getHitPoints() <= berserkLimit)
    return standardDamage * 3 / 2;
  return standardDamage;
}

unsigned Monster::getPhysicalResistPercent() const
{
  return defence.getPhysicalResistPercent();
}

unsigned Monster::getMagicalResistPercent() const
{
  return defence.getMagicalResistPercent();
}

unsigned Monster::predictDamageTaken(unsigned attackerDamageOutput, DamageType damageType) const
{
  return defence.predictDamageTaken(attackerDamageOutput, damageType, status.getBurnStackSize());
}

void Monster::takeDamage(unsigned attackerDamageOutput, DamageType damageType)
{
  stats.loseHitPoints(HitPoints{predictDamageTaken(attackerDamageOutput, damageType)});
  status.setSlowed(false);
  status.setBurn(0);
}

void Monster::takeFireballDamage(unsigned casterLevel, unsigned damageMultiplier)
{
  const auto damagePoints = casterLevel * damageMultiplier;
  stats.loseHitPoints(HitPoints{predictDamageTaken(damagePoints, DamageType::Magical)});
  status.setSlowed(false);
  burn(casterLevel * 2);
}

void Monster::takeBurningStrikeDamage(unsigned attackerDamageOutput, unsigned casterLevel, DamageType damageType)
{
  stats.loseHitPoints(HitPoints{predictDamageTaken(attackerDamageOutput, damageType)});
  status.setSlowed(false);
  burn(casterLevel * 2);
}

void Monster::takeManaShieldDamage(unsigned casterLevel)
{
  // Mana Shield damage against magical resistance is rounded down (usually resisted damage is rounded down)
  stats.loseHitPoints(HitPoints{casterLevel * (100 - getMagicalResistPercent()) / 100});
}

void Monster::receiveCrushingBlow()
{
  const auto hp = stats.getHitPoints();
  const auto crushed = HitPoints{stats.getHitPointsMax().get() * 3u / 4u};
  if (hp > crushed)
    stats.loseHitPoints(hp - crushed);
  status.setSlowed(false);
  status.setBurn(0);
}

void Monster::recover(unsigned nSquares)
{
  if (isDefeated())
    return;
  auto recoverPoints = nSquares * (getLevel() - (status.isBurning() ? 1 : 0));
  if (has(MonsterTrait::FastRegen))
    recoverPoints *= 2;
  if (status.isPoisoned())
  {
    const auto poison = status.getPoisonAmount();
    if (poison >= recoverPoints)
    {
      status.setPoison(static_cast<uint16_t>(poison - recoverPoints));
      recoverPoints = 0;
    }
    else
    {
      status.setPoison(0);
      recoverPoints -= poison;
    }
  }
  if (recoverPoints > 0)
    stats.healHitPoints(HitPoints{recoverPoints}, false);
}

void Monster::burn(unsigned nMaxStacks)
{
  if (status.getBurnStackSize() < clampedTo<uint8_t>(nMaxStacks))
    status.setBurn(static_cast<uint8_t>(status.getBurnStackSize() + 1));
}

void Monster::burnMax(unsigned nMaxStacks)
{
  status.setBurn(clampedTo<uint8_t>(nMaxStacks));
}

void Monster::burnDown()
{
  if (status.getBurnStackSize() > 0)
  {
    const auto resist = HitPoints{status.getBurnStackSize() * getMagicalResistPercent() / 100};
    stats.loseHitPoints(HitPoints{status.getBurnStackSize()} - resist);
    status.setSlowed(false);
    status.setBurn(0);
  }
}

bool Monster::poison(unsigned addedPoisonAmount)
{
  if (has(MonsterTrait::Undead))
    return false;
  status.setPoison(clampedTo<uint16_t>(status.getPoisonAmount() + addedPoisonAmount));
  return true;
}

void Monster::slow()
{
  status.setSlowed(true);
}

void Monster::erodeResitances()
{
  const auto physicalResist = defence.getPhysicalResistPercent();
  const auto magicalResist = defence.getMagicalResistPercent();
  defence.setPhysicalResistPercent(physicalResist > 3 ? physicalResist - 3 : 0);
  defence.setMagicalResistPercent(magicalResist > 3 ? magicalResist - 3 : 0);
}

void Monster::petrify()
{
  die();
}

void Monster::die()
{
  status.setBurn(false);
  status.setSlowed(false);
  stats.set(DeathProtection{0});
  stats.loseHitPoints(stats.getHitPoints());
}

void Monster::corrode(unsigned amount)
{
  status.setCorroded(getCorroded() + amount);
  defence.setCorrosion(getCorroded());
}

void Monster::zot()
{
  if (!status.isZotted())
  {
    status.setZotted();
    stats.setHitPointsMax(HitPoints{stats.getHitPointsMax().get() / 2});
  }
}

void Monster::makeWickedSick()
{
  auto level = Level{getLevel()};
  if (!status.isWickedSick() && level.increase())
  {
    const auto type = stats.getType();
    const bool hasStandardName = name == makeMonsterName(type, level);
    status.setWickedSick();
    stats = MonsterStats(stats.getType(), level, stats.getDungeonMultiplier());
    if (hasStandardName)
      name = makeMonsterName(type, level);
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

unsigned Monster::getBurnStackSize() const
{
  return status.getBurnStackSize();
}

unsigned Monster::getPoisonAmount() const
{
  return status.getPoisonAmount();
}

unsigned Monster::getDeathProtection() const
{
  return stats.getDeathProtection().get();
}

unsigned Monster::getCorroded() const
{
  return status.getCorroded();
}

DamageType Monster::damageType() const
{
  return traits.has(MonsterTrait::MagicalAttack) ? DamageType::Magical : DamageType::Physical;
}

bool Monster::has(MonsterTrait trait) const
{
  return traits.has(trait);
}

unsigned Monster::getDeathGazePercent() const
{
  return traits.getDeathGazePercent();
}

unsigned Monster::getLifeStealPercent() const
{
  return traits.getLifeStealPercent();
}

void Monster::changePhysicalResist(int deltaPercent)
{
  defence.setPhysicalResistPercent(defence.getPhysicalResistPercent() + deltaPercent);
}

void Monster::changeMagicResist(int deltaPercent)
{
  defence.setMagicalResistPercent(defence.getMagicalResistPercent() + deltaPercent);
}

void Monster::applyTikkiTookiBoost()
{
  traits.applyTikkiTookiBoost();
}

using namespace std::string_literals;

std::vector<std::string> describe(const Monster& monster)
{
  if (monster.isDefeated())
    return {monster.getName() + " defeated."};

  std::vector description{monster.getName(),
                          std::to_string(monster.getHitPoints()) + "/" + std::to_string(monster.getHitPointsMax()) +
                              " HP",
                          std::to_string(monster.getDamage()) + " damage"};

  auto checkTrait = [&](MonsterTrait trait) {
    if (monster.has(trait))
      description.emplace_back(toString(trait));
  };

  if (!monster.isSlowed())
  {
    checkTrait(MonsterTrait::Blinks);
    checkTrait(MonsterTrait::FirstStrike);
    if (monster.has(MonsterTrait::Retaliate))
      description.emplace_back("Retaliate: Fireball");
  }
  checkTrait(MonsterTrait::Cowardly);
  checkTrait(MonsterTrait::FastRegen);
  checkTrait(MonsterTrait::MagicalAttack);
  if (monster.getPhysicalResistPercent() > 0)
    description.emplace_back("Physical resist "s + std::to_string(monster.getPhysicalResistPercent()) + "%");
  if (monster.getMagicalResistPercent() > 0)
    description.emplace_back("Magical resist "s + std::to_string(monster.getMagicalResistPercent()) + "%");
  checkTrait(MonsterTrait::Poisonous);
  checkTrait(MonsterTrait::ManaBurn);
  checkTrait(MonsterTrait::CurseBearer);
  checkTrait(MonsterTrait::Corrosive);
  if (monster.has(MonsterTrait::Weakening))
    description.emplace_back("Weakening Blow");
  if (monster.getDeathGazePercent() > 0)
    description.emplace_back("Death Gaze "s + std::to_string(monster.getDeathGazePercent()) + "%");
  if (monster.getDeathProtection() > 0)
    description.emplace_back("Death protection (x"s + std::to_string(monster.getDeathProtection()) + ")");
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
  checkTrait(MonsterTrait::Undead);
  checkTrait(MonsterTrait::Bloodless);
  if (!monster.grantsXP())
    description.emplace_back("No Experience");

  return description;
}
