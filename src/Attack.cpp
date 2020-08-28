#include "Attack.hpp"

#include "Hero.hpp"
#include "Monster.hpp"

Attack::Attack()
  : damage(5)
  , damageBonusPercent(0)
{
}

Attack::Attack(int damage, int damageBonusPercent)
  : damage(damage)
  , damageBonusPercent(damageBonusPercent)
{
}

AttackBehaviour* Attack::clone() const
{
  return new Attack(*this);
}

int Attack::getBaseDamage() const
{
  return damage;
}

void Attack::changeBaseDamage(int deltaDamagePoints)
{
  damage += deltaDamagePoints;
  if (damage < 0)
    damage = 0;
}

void Attack::levelGainedUpdate()
{
  damage += 5;
}

int Attack::getDamageBonusPercent() const
{
  return damageBonusPercent;
}

void Attack::changeDamageBonusPercent(int deltaDamageBonusPercent)
{
  damageBonusPercent += deltaDamageBonusPercent;
}

int Attack::getDamage() const
{
  return getBaseDamage() * (100 + getDamageBonusPercent()) / 100;
}

bool Attack::hasInitiativeVersus(const Hero& hero, const Monster& monster) const
{
  // TODO Override for class-specific rules, e.g. Rogue, Assassin
  if (hero.hasStatus(HeroStatus::Reflexes))
    return true;

  const bool heroFast = hero.hasStatus(HeroStatus::FirstStrike) && !hero.hasStatus(HeroStatus::SlowStrike);
  const bool monsterFast = monster.hasFirstStrike() && !monster.isStunned();
  if (heroFast || monsterFast)
    return !monsterFast;

  const bool heroSlow = !hero.hasStatus(HeroStatus::FirstStrike) && hero.hasStatus(HeroStatus::SlowStrike);
  const bool monsterSlow = monster.isStunned();
  if (heroSlow || monsterSlow)
    return !heroSlow;

  return hero.getLevel() > monster.getLevel();
}
