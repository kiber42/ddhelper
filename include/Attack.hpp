#pragma once

#include "AttackBehaviour.hpp"

class Attack : public AttackBehaviour
{
public:
  Attack();
  Attack(int damage, int damageBonusPercent);

  int getBaseDamage() const;
  void changeBaseDamage(int deltaDamagePoints);
  void levelGainedUpdate();

  int getDamageBonusPercent() const;
  void changeDamageBonusPercent(int deltaDamageBonusPercent);

  int getDamage() const;

  bool hasInitiativeVersus(const Hero &, const Monster &) const;

private:
  int damage;
  int damageBonusPercent;
};
