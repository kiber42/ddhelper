#include "engine/Jehora.hpp"

#include "engine/Hero.hpp"

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

unsigned Jehora::initialPietyBonus(unsigned heroLevel, bool isPessimist)
{
  unsigned multiplier;
  if (!isPessimist)
  {
    std::uniform_int_distribution<unsigned> initialBonus(70, 100);
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

unsigned Jehora::operator()()
{
  // Randomly award 2-4 XP or apply random punishment
  std::uniform_int_distribution<unsigned> happinessRoll(1, 15);
  std::uniform_int_distribution<unsigned> pietyRoll(2, 4);
  if (happinessRoll(generator) <= happiness)
  {
    happiness = std::max(happiness - 1, 5u);
    return pietyRoll(generator);
  }
  // a punishment is to be applied
  happiness = std::min(happiness + 1, 14u);
  return 0;
}

void Jehora::applyRandomPunishment(Hero& hero)
{
  if (hero.has(Boon::Petition))
    return;
  // By design, Jehora's punishments (like poisoning or mana burn) cannot trigger chain reactions
  // (i.e. he will never be upset about them), and none of his punishments affect other monsters.
  // It is therefore okay to pass an empty vector as second argument to add.
  Monsters ignore;
  std::uniform_int_distribution<> punishmentRoll(0, 10);
  unsigned rerolls = 0;
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
          hero.add(HeroDebuff::Poisoned, ignore);
          ++thresholdPoison;
          return;
        }
        break;
      case 1:
        if (rerolls >= thresholdManaBurn)
        {
          hero.add(HeroDebuff::ManaBurned, ignore);
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
      break;
    }
    case 1:
      if (rerolls >= thresholdWeakened)
      {
        hero.add(HeroDebuff::Weakened, ignore);
        ++thresholdWeakened;
        return;
      }
      break;
    case 2:
      if (rerolls >= thresholdCorrosion)
      {
        hero.add(HeroDebuff::Corroded, ignore);
        ++thresholdCorrosion;
      }
      break;
    case 3:
      if (rerolls >= thresholdCursed)
      {
        hero.add(HeroDebuff::Cursed, ignore);
        ++thresholdCursed;
        return;
      }
      break;
    }
    ++rerolls;
  }
}
