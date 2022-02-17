#include "engine/Monster.hpp"

#include "engine/Clamp.hpp"
#include "engine/MonsterStats.hpp"
#include "engine/MonsterTypes.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
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

Monster::Monster(MonsterType type, Level level, DungeonMultiplier dungeonMultiplier)
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

Monster::Monster(MonsterStats stats, Defence defence, MonsterTraits traits)
  : Monster("Monster level " + std::to_string(stats.getLevel().get()),
            std::move(stats),
            std::move(defence),
            std::move(traits))
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
  return isEnraged() ? standardDamage * 3 / 2 : standardDamage;
}

unsigned Monster::getPhysicalResistPercent() const
{
  return defence.getPhysicalResist().in_percent();
}

unsigned Monster::getMagicalResistPercent() const
{
  return defence.getMagicalResist().in_percent();
}

unsigned Monster::predictDamageTaken(unsigned attackerDamageOutput, DamageType damageType) const
{
  return defence.predictDamageTaken(DamagePoints{attackerDamageOutput}, damageType, status.getBurnStackSize()).get();
}

void Monster::takeDamage(unsigned attackerDamageOutput, DamageType damageType)
{
  stats.loseHitPoints(HitPoints{predictDamageTaken(attackerDamageOutput, damageType)});
  status.setSlowed(false);
  status.set(0_burn);
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
  const auto crushed = stats.getHitPointsMax().percentage(75);
  if (hp > crushed)
    stats.loseHitPoints(hp - crushed);
  status.setSlowed(false);
  status.set(0_burn);
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
    if (poison.get() >= recoverPoints)
    {
      status.set(poison - PoisonAmount{recoverPoints});
      recoverPoints = 0;
    }
    else
    {
      status.set(0_poison);
      recoverPoints -= poison.get();
    }
  }
  if (recoverPoints > 0)
    stats.healHitPoints(HitPoints{recoverPoints}, false);
}

void Monster::burn(unsigned nMaxStacks)
{
  if (status.getBurnStackSize() < BurnStackSize{clampedTo<uint8_t>(nMaxStacks)})
    status.set(status.getBurnStackSize() + 1_burn);
}

void Monster::burnMax(unsigned nMaxStacks)
{
  status.set(BurnStackSize{clampedTo<uint8_t>(nMaxStacks)});
}

void Monster::burnDown()
{
  if (status.getBurnStackSize() > 0_burn)
  {
    const auto burnPoints = HitPoints{status.getBurnStackSize().get()};
    const auto resist = burnPoints.percentage(getMagicalResistPercent());
    stats.loseHitPoints(burnPoints - resist);
    status.setSlowed(false);
    status.set(0_burn);
  }
}

bool Monster::poison(unsigned addedPoisonAmount)
{
  if (has(MonsterTrait::Undead))
    return false;
  status.set(status.getPoisonAmount() + PoisonAmount{addedPoisonAmount});
  return true;
}

void Monster::slow()
{
  status.setSlowed(true);
}

void Monster::erodeResitances()
{
  const auto physicalResist = defence.getPhysicalResist();
  const auto magicalResist = defence.getMagicalResist();
  defence.set(physicalResist > 3_physicalresist ? physicalResist - 3_physicalresist : 0_physicalresist);
  defence.set(magicalResist > 3_magicalresist ? magicalResist - 3_magicalresist : 0_magicalresist);
}

void Monster::petrify(Resources& resources)
{
  die();
  ++resources().numWalls;
}

void Monster::die()
{
  status.set(0_burn);
  status.setSlowed(false);
  stats.set(DeathProtection{0});
  stats.loseHitPoints(stats.getHitPoints());
}

void Monster::corrode(unsigned amount)
{
  const auto newCorrosion = status.getCorroded() + CorrosionAmount{amount};
  status.set(newCorrosion);
  defence.set(newCorrosion);
}

void Monster::makeCorrosive()
{
  traits.addCorrosive();
}

void Monster::makeWickedSick()
{
  auto level = Level{getLevel()};
  if (!isWickedSick() && level.increase())
  {
    const auto type = stats.getType();
    const bool hasStandardName = name == makeMonsterName(type, level);
    traits.addWickedSick();
    stats = MonsterStats(stats.getType(), level, stats.getDungeonMultiplier());
    if (hasStandardName)
      name = makeMonsterName(type, level);
  }
}

void Monster::zot()
{
  if (!isZotted())
  {
    traits.addZotted();
    stats.setHitPointsMax(stats.getHitPointsMax() / 2);
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
  return traits.has(MonsterTrait::Zotted);
}

bool Monster::isWickedSick() const
{
  return traits.has(MonsterTrait::WickedSick);
}

unsigned Monster::getBurnStackSize() const
{
  return status.getBurnStackSize().get();
}

unsigned Monster::getPoisonAmount() const
{
  return status.getPoisonAmount().get();
}

unsigned Monster::getDeathProtection() const
{
  return stats.getDeathProtection().get();
}

unsigned Monster::getCorroded() const
{
  return status.getCorroded().get();
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
  return traits.deathGaze().in_percent();
}

unsigned Monster::getLifeStealPercent() const
{
  return traits.lifeSteal().in_percent();
}

unsigned Monster::getBerserkPercent() const
{
  return traits.berserk().in_percent();
}

bool Monster::isEnraged() const
{
  const auto berserkLimit = getHitPointsMax() * getBerserkPercent() / 100;
  return berserkLimit > 0 && getHitPoints() <= berserkLimit;
}

void Monster::changePhysicalResist(int deltaPercent)
{
  defence.changePhysicalResistPercent(deltaPercent);
}

void Monster::changeMagicResist(int deltaPercent)
{
  defence.changeMagicalResistPercent(deltaPercent);
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

  auto checkTrait = [&](MonsterTrait trait, std::string label = "") {
    if (monster.has(trait))
    {
      if (label.empty())
        description.emplace_back(toString(trait));
      else
        description.emplace_back(std::move(label));
    }
  };
  auto checkIntensity = [&](unsigned intensity, std::string prefix, std::string suffix)
  {
    if (intensity > 0)
      description.emplace_back(std::move(prefix) + std::to_string(intensity) + std::move(suffix));
  };

  if (!monster.isSlowed())
  {
    checkTrait(MonsterTrait::Blinks);
    checkTrait(MonsterTrait::FirstStrike);
    checkTrait(MonsterTrait::Retaliate, "Retaliate: Fireball");
  }
  const auto berserkPercent = monster.getBerserkPercent();
  if (berserkPercent > 0)
  {
    if (monster.isEnraged())
      description.emplace_back("Enraged");
    else
      description.emplace_back("Berserks at " + std::to_string(berserkPercent) + "%");
  }
  checkTrait(MonsterTrait::Cowardly);
  checkTrait(MonsterTrait::FastRegen);
  checkTrait(MonsterTrait::MagicalAttack);
  checkIntensity(monster.getPhysicalResistPercent(), "Physical resist ", "%");
  checkIntensity(monster.getMagicalResistPercent(), "Magical resist ", "%");
  checkTrait(MonsterTrait::Poisonous);
  checkTrait(MonsterTrait::ManaBurn);
  checkTrait(MonsterTrait::CurseBearer);
  checkTrait(MonsterTrait::Corrosive);
  checkTrait(MonsterTrait::Weakening, "Weakening Blow");
  checkIntensity(monster.getDeathGazePercent(), "Death Gaze ", "%");
  checkIntensity(monster.getDeathProtection(), "Death protection (x", ")");
  checkIntensity(monster.getLifeStealPercent(), "Life Steal ", "%");
  checkIntensity(monster.getBurnStackSize(), "Burning (burn stack size ", ")");
  checkIntensity(monster.getPoisonAmount(), "Poisoned (amount: ", ")");
  if (monster.isSlowed())
    description.emplace_back("Slowed");
  if (monster.isZotted())
    description.emplace_back("Zotted");
  if (monster.isWickedSick())
    description.emplace_back("Wicked Sick");
  checkIntensity(monster.getCorroded(), "Corroded (x", ")");
  checkTrait(MonsterTrait::Undead);
  checkTrait(MonsterTrait::Bloodless);
  if (!monster.grantsXP())
    description.emplace_back("No Experience");

  return description;
}

HiddenMonster::HiddenMonster(Monster monster)
  : monster(std::move(monster))
{
}

HiddenMonster::HiddenMonster(Level level, DungeonMultiplier dungeonMultiplier, bool includeAdvanced)
  : monster(MonsterDescription{level, dungeonMultiplier, includeAdvanced})
{
}

template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

Level HiddenMonster::getLevel() const
{
  return std::visit(overloaded{
                        [](const Monster& monster) { return Level{monster.getLevel()}; },
                        [](const MonsterDescription& description) { return description.level; },
                    },
                    monster);
}

Monster HiddenMonster::reveal(std::mt19937& generator)
{
  if (auto revealedMonsterPtr = std::get_if<Monster>(&monster))
  {
    auto revealedMonster = std::move(*revealedMonsterPtr);
    monster = MonsterDescription{Level{revealedMonster.getLevel()}};
    return revealedMonster;
  }
  const auto& description = std::get<MonsterDescription>(monster);
  const auto type = MonsterType(std::uniform_int_distribution<unsigned char>(
      0, (unsigned char)(description.includeAdvanced ? MonsterType::Last : MonsterType::LastBasic))(generator));
  return Monster(type, description.level, description.dungeonMultiplier);
}
