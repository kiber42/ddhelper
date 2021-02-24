#include "Jehora.hpp"

#include "Hero.hpp"

Jehora::Jehora()
  : generator(std::random_device{}())
  , happiness(14)
  , thresholdPoison(0)
  , thresholdManaBurn(1)
  , thresholdHealthLoss(1)
  , thresholdWeakened(0)
  , thresholdCorrosion(0)
  , thresholdCursed(0)
{
  // Jehora's reward / punish mechanics gratefully implemented based on DDwiki:
  // http://www.qcfdesign.com/wiki/DesktopDungeons/index.php?title=Jehora_Jeheyu
}

int Jehora::initialPietyBonus(int heroLevel, bool isPessimist)
{
  int multiplier;
  if (!isPessimist)
  {
    std::uniform_int_distribution<> initialBonus(70, 100);
    multiplier = initialBonus(generator);
  }
  else
    multiplier = 70;
  return heroLevel * heroLevel * multiplier / 100;
}

bool Jehora::lastChanceSuccessful(int remainingPiety)
{
  // TODO: Currently simplified to avoid dealing with randomness
  return remainingPiety >= 50;
}

int Jehora::operator()()
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

void Jehora::applyRandomPunishment(Hero& hero)
{
  if (hero.has(Boon::Petition))
    return;
  // By design, Jehora's punishments (like poisoning or mana burn) cannot trigger chain reactions
  // (i.e. he will never be upset about them), and none of his punishments affect other monsters.
  // It is therefore okay to pass an empty vector as second argument to addStatus.
  Monsters ignore;
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
          hero.addStatus(HeroDebuff::Poisoned, ignore);
          ++thresholdPoison;
          return;
        }
        break;
      case 1:
        if (rerolls >= thresholdManaBurn)
        {
          hero.addStatus(HeroDebuff::ManaBurned, ignore);
          ++thresholdManaBurn;
          return;
        }
        break;
      case 2:
        if (rerolls >= thresholdHealthLoss)
        {
          hero.loseHitPointsOutsideOfFight(hero.getHitPoints() - 1, ignore);
          ++thresholdHealthLoss;
          return;
        }
      }
    }
    case 1:
      if (rerolls >= thresholdWeakened)
      {
        hero.addStatus(HeroDebuff::Weakened, ignore);
        ++thresholdWeakened;
        return;
      }
      break;
    case 2:
      if (rerolls >= thresholdCorrosion)
      {
        hero.addStatus(HeroDebuff::Corroded, ignore);
        ++thresholdCorrosion;
      }
      break;
    case 3:
      if (rerolls >= thresholdCursed)
      {
        hero.addStatus(HeroDebuff::Cursed, ignore);
        ++thresholdCursed;
        return;
      }
      break;
    }
    ++rerolls;
  }
}
