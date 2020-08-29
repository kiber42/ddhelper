#pragma once

#include "AttackBehaviour.hpp"

class Attack : public AttackBehaviour
{
public:
  Attack();
  Attack(int damage, int damageBonusPercent);

  AttackBehaviour* clone() const override;

  int getBaseDamage() const override;
  void changeBaseDamage(int deltaDamagePoints) override;
  void levelGainedUpdate() override;

  int getDamageBonusPercent() const override;
  void changeDamageBonusPercent(int deltaDamageBonusPercent) override;

  bool hasInitiativeVersus(const Hero&, const Monster&) const override;

private:
  int damage;
  int damageBonusPercent;
};
