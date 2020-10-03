#pragma once

enum class Item
{
  // Basic Items
  BadgeOfHonour,
  BloodySigil,
  FineSword,
  PendantOfHealth,
  PendantOfMana,
  Spoon,
  TowerShield,
  TrollHeart,
};

constexpr const char* toString(Item item)
{
  switch (item)
  {
  case Item::BadgeOfHonour:
    return "Badge of Honour";
  case Item::BloodySigil:
    return "Bloody Sigil";
  case Item::FineSword:
    return "Fine Sword";
  case Item::PendantOfHealth:
    return "Pendant of Health";
  case Item::PendantOfMana:
    return "Pendant of Mana";
  case Item::Spoon:
    return "Spoon";
  case Item::TowerShield:
    return "Tower Shield";
  case Item::TrollHeart:
    return "Troll Heart";
  }
}

constexpr bool isSmall(Item item)
{
  return item == Item::Spoon;
}

constexpr int price(Item item)
{
  switch (item)
  {
  case Item::BadgeOfHonour:
    return 18;
  case Item::BloodySigil:
    return 8;
  case Item::FineSword:
    return 15;
  case Item::PendantOfHealth:
    return 15;
  case Item::PendantOfMana:
    return 12;
  case Item::Spoon:
    return 1;
  case Item::TowerShield:
    return 14;
  case Item::TrollHeart:
    return 16;
  }
}

constexpr int conversionPointsInitial(Item item)
{
  switch (item)
  {
  case Item::BadgeOfHonour:
    return 40;
  case Item::BloodySigil:
    return 45;
  case Item::FineSword:
    return 35;
  case Item::PendantOfHealth:
    return 35;
  case Item::PendantOfMana:
    return 35;
  case Item::Spoon:
    return 1;
  case Item::TowerShield:
    return 35;
  case Item::TrollHeart:
    return 55;
  }
}

constexpr bool canGroup(Item item)
{
  return false;
}
