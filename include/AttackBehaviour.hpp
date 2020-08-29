#pragma once

class Hero;
class Monster;

#include <vector>

class AttackBehaviour
{
public:
  virtual ~AttackBehaviour() = default;

  virtual AttackBehaviour* clone() const = 0;

  virtual int getBaseDamage() const = 0;
  virtual void changeBaseDamage(int deltaDamagePoints) = 0;
  virtual void levelGainedUpdate() = 0;

  virtual int getDamageBonusPercent() const = 0;
  virtual void changeDamageBonusPercent(int deltaDamageBonusPercent) = 0;

  virtual bool hasInitiativeVersus(const Hero &, const Monster &) const = 0;
};
