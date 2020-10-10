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

int Jehora::initialPietyBonus(int heroLevel)
{
  std::uniform_int_distribution<> initialBonus(70, 100);
  return initialBonus(generator) * heroLevel * heroLevel / 100;
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
