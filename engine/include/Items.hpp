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
  // Potions
  HealthPotion,
  ManaPotion,
  FortitudeTonic,
  BurnSalve,
  StrengthPotion,
  Schadenfreude,
  QuicksilverPotion,
  ReflexPotion,
  CanOfWhupaz,
  // God Items
  PrayerBead,
  EnchantedPrayerBead,
  Skullpicker,
  Wereward,
  Gloat,
  Will
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

  case Item::HealthPotion:
    return "Health Potion";
  case Item::ManaPotion:
    return "Mana Potion";
  case Item::FortitudeTonic:
    return "Fortitude Tonic";
  case Item::BurnSalve:
    return "Burn Salve";
  case Item::StrengthPotion:
    return "Strength Potion";
  case Item::Schadenfreude:
    return "Schadenfreude";
  case Item::QuicksilverPotion:
    return "Quicksilver Potion";
  case Item::ReflexPotion:
    return "Reflex Potion";
  case Item::CanOfWhupaz:
    return "Can of Whupaz";

  case Item::PrayerBead:
    return "Prayer Bead";
  case Item::EnchantedPrayerBead:
    return "Enchanted Prayer Bead";

  case Item::Skullpicker:
    return "Skullpicker";
  case Item::Wereward:
    return "Wereward";
  case Item::Gloat:
    return "Gloat";
  case Item::Will:
    return "Will";
  }
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

  case Item::HealthPotion:
    return 10;
  case Item::ManaPotion:
    return 10;
  case Item::FortitudeTonic:
    return 8;
  case Item::BurnSalve:
    return 8;
  case Item::StrengthPotion:
    return 15;
  case Item::Schadenfreude:
    return 15;
  case Item::QuicksilverPotion:
    return 15;
  case Item::ReflexPotion:
    return 15;
  case Item::CanOfWhupaz:
    return 20;

  case Item::PrayerBead:
  case Item::EnchantedPrayerBead:
  case Item::Skullpicker:
  case Item::Wereward:
  case Item::Gloat:
  case Item::Will:
    return 0;
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

  case Item::HealthPotion:
    return 10;
  case Item::ManaPotion:
    return 10;
  case Item::FortitudeTonic:
    return 10;
  case Item::BurnSalve:
    return 10;
  case Item::StrengthPotion:
    return 10;
  case Item::Schadenfreude:
    return 10;
  case Item::QuicksilverPotion:
    return 10;
  case Item::ReflexPotion:
    return 10;
  case Item::CanOfWhupaz:
    return 10;

  case Item::PrayerBead:
    return -1;
  case Item::EnchantedPrayerBead:
    return -1;

  case Item::Skullpicker:
  case Item::Wereward:
  case Item::Gloat:
  case Item::Will:
    return 60;
  }
}

constexpr bool isPotion(Item item)
{
  return item == Item::HealthPotion || item == Item::ManaPotion || item == Item::FortitudeTonic ||
         item == Item::BurnSalve || item == Item::StrengthPotion || item == Item::Schadenfreude ||
         item == Item::QuicksilverPotion || item == Item::ReflexPotion || item == Item::CanOfWhupaz;
}

constexpr bool isSmall(Item item)
{
  return isPotion(item) || item == Item::Spoon || item == Item::PrayerBead || item == Item::EnchantedPrayerBead;
}

constexpr bool canGroup(Item item)
{
  return isPotion(item);
}
