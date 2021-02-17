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
  // Quest Items
  PiercingWand,
  RockHeart,
  DragonSoul,
  FireHeart,
  CrystalBall,
  WitchalokPendant,
  BattlemageRing,
  HerosHelm,
  Platemail,
  Whurrgarbl,
  Trisword,
  BalancedDagger,
  GlovesOfMidas,
  VenomDagger,
  StoneSigil,
  MartyrWraps,
  AgnosticCollar,
  MagePlate,
  BlueBead,
  VampiricBlade,
  ViperWard,
  SoulOrb,
  // Elite Items
  KegOfHealth,
  KegOfMana,
  ElvenBoots,
  DwarvenGauntlets,
  AmuletOfYendor,
  OrbOfZot,
  AlchemistScroll,
  WickedGuitar,
  LastShopItem = WickedGuitar,
  // Blacksmith Items
  BearMace,
  PerseveranceBadge,
  ReallyBigSword,
  Shield,
  SlayerWand,
  Sword,
  // Potions
  FreeHealthPotion,
  FreeManaPotion,
  HealthPotion,
  ManaPotion,
  FortitudeTonic,
  BurnSalve,
  StrengthPotion,
  Schadenfreude,
  QuicksilverPotion,
  ReflexPotion,
  CanOfWhupaz,
  // Alchemist Seals
  CompressionSeal,
  TransmutationSeal,
  TranslocationSeal,
  // Misc Items
  ShopScroll,
  PatchesTheTeddy,
  // Subdungeon Items, Lockerable
  Gorgward,
  // Boss Rewards
  FabulousTreasure,
  DragonShield,
  NamtarsWard,
  AvatarsCodex,
  NagaCauldron,
  SensationStone,
  // God Items
  PrayerBead,
  EnchantedPrayerBead,
  Skullpicker,
  Wereward,
  Gloat,
  Will,
  // Puzzle Items
  TikkisCharm,
  Last = TikkisCharm,
  // Internal use
  AnyPotion
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

  case Item::PiercingWand:
    return "Piercing Wand";
  case Item::RockHeart:
    return "Rock Heart";
  case Item::DragonSoul:
    return "Dragon Soul";
  case Item::FireHeart:
    return "Fire Heart";
  case Item::CrystalBall:
    return "Crystal Ball";
  case Item::WitchalokPendant:
    return "Witchalok Pendant";
  case Item::BattlemageRing:
    return "Battlemage Ring";
  case Item::HerosHelm:
    return "Hero's Helm";
  case Item::Platemail:
    return "Platemail";
  case Item::Whurrgarbl:
    return "Whurrgarbl";
  case Item::Trisword:
    return "Trisword";
  case Item::BalancedDagger:
    return "Balanced Dagger";
  case Item::GlovesOfMidas:
    return "Gloves Of Midas";
  case Item::VenomDagger:
    return "Venom Dagger";
  case Item::StoneSigil:
    return "Stone Sigil";
  case Item::MartyrWraps:
    return "Martyr Wraps";
  case Item::AgnosticCollar:
    return "Agnostic Collar";
  case Item::MagePlate:
    return "Mage Plate";
  case Item::BlueBead:
    return "Blue Bead";
  case Item::VampiricBlade:
    return "Vampiric Blade";
  case Item::ViperWard:
    return "Viper Ward";
  case Item::SoulOrb:
    return "Soul Orb";

  case Item::KegOfHealth:
    return "Keg Of Health";
  case Item::KegOfMana:
    return "Keg Of Mana";
  case Item::ElvenBoots:
    return "Elven Boots";
  case Item::DwarvenGauntlets:
    return "Dwarven Gauntlets";
  case Item::AmuletOfYendor:
    return "Amulet of Yendor";
  case Item::OrbOfZot:
    return "Orb of Zot";
  case Item::AlchemistScroll:
    return "Alchemist Scroll";
  case Item::WickedGuitar:
    return "Wicked Guitar";

  case Item::BearMace:
    return "Bear Mace";
  case Item::PerseveranceBadge:
    return "Perseverance Badge";
  case Item::ReallyBigSword:
    return "Really Big Sword";
  case Item::Shield:
    return "Shield";
  case Item::SlayerWand:
    return "Slayer Wand";
  case Item::Sword:
    return "Sword";

  case Item::FreeHealthPotion:
  case Item::HealthPotion:
    return "Health Potion";
  case Item::FreeManaPotion:
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

  case Item::CompressionSeal:
    return "Compression Seal";
  case Item::TransmutationSeal:
    return "Transmutation Seal";
  case Item::TranslocationSeal:
    return "Translocation Seal";
  case Item::ShopScroll:
    return "Shop Scroll";
  case Item::PatchesTheTeddy:
    return "Patches the Teddy";

  case Item::Gorgward:
    return "Gorgward";

  case Item::FabulousTreasure:
    return "Fabulous Treasure";
  case Item::DragonShield:
    return "Dragon Shield";
  case Item::NamtarsWard:
    return "Namtar's Ward";
  case Item::AvatarsCodex:
    return "Avatar's Codex";
  case Item::NagaCauldron:
    return "Naga Cauldron";
  case Item::SensationStone:
    return "Sensation Stone";

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

  case Item::TikkisCharm:
    return "Tikki's Charm";

  case Item::AnyPotion:
    return "";
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

  case Item::PiercingWand:
    return 13;
  case Item::RockHeart:
    return 14;
  case Item::DragonSoul:
    return 23;
  case Item::FireHeart:
    return 20;
  case Item::CrystalBall:
    return 15;
  case Item::WitchalokPendant:
    return 19;
  case Item::BattlemageRing:
    return 25;
  case Item::HerosHelm:
    return 18;
  case Item::Platemail:
    return 23;
  case Item::Whurrgarbl:
    return 15;
  case Item::Trisword:
    return 12;
  case Item::BalancedDagger:
    return 12;
  case Item::GlovesOfMidas:
    return 10;
  case Item::VenomDagger:
    return 16;
  case Item::StoneSigil:
    return 14;
  case Item::MartyrWraps:
    return 15;
  case Item::AgnosticCollar:
    return 25;
  case Item::MagePlate:
    return 20;
  case Item::BlueBead:
    return 5;
  case Item::VampiricBlade:
    return 25;
  case Item::ViperWard:
    return 16;
  case Item::SoulOrb:
    return 16;

  case Item::KegOfHealth:
    return 25;
  case Item::KegOfMana:
    return 25;
  case Item::ElvenBoots:
    return 35;
  case Item::DwarvenGauntlets:
    return 35;
  case Item::AmuletOfYendor:
    return 45;
  case Item::OrbOfZot:
    return 45;
  case Item::AlchemistScroll:
    return 13;
  case Item::WickedGuitar:
    return 11;

  case Item::BearMace:
    return 12;
  case Item::PerseveranceBadge:
    return 15;
  case Item::ReallyBigSword:
    return 12;
  case Item::Shield:
    return 15;
  case Item::SlayerWand:
    return 5;
  case Item::Sword:
    return 25;

  case Item::FreeHealthPotion:
    return 0;
  case Item::FreeManaPotion:
    return 0;
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

  case Item::CompressionSeal:
    return 16;
  case Item::TransmutationSeal:
    return 36;
  case Item::TranslocationSeal:
    return 43;
  case Item::ShopScroll:
    return 10;
  case Item::PatchesTheTeddy:
    return 0;

  case Item::Gorgward:
    return 18;

  case Item::FabulousTreasure:
    return 95;
  case Item::DragonShield:
    return 23;
  case Item::NamtarsWard:
    return 50;
  case Item::AvatarsCodex:
    return 35;
  case Item::NagaCauldron:
    return 12;
  case Item::SensationStone:
    return 25;

  case Item::PrayerBead:
  case Item::EnchantedPrayerBead:
    return -1; // cannot be transmuted

  case Item::Skullpicker:
  case Item::Wereward:
  case Item::Gloat:
  case Item::Will:
    return 0;

  case Item::TikkisCharm:
    return 1;
  case Item::AnyPotion:
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

  case Item::PiercingWand:
    return 30;
  case Item::RockHeart:
    return 60;
  case Item::DragonSoul:
    return 50;
  case Item::FireHeart:
    return 12;
  case Item::CrystalBall:
    return 50;
  case Item::WitchalokPendant:
    return 30;
  case Item::BattlemageRing:
    return 20;
  case Item::HerosHelm:
    return 50;
  case Item::Platemail:
    return 40;
  case Item::Whurrgarbl:
    return 45;
  case Item::Trisword:
    return 35;
  case Item::BalancedDagger:
    return 44;
  case Item::GlovesOfMidas:
    return 45;
  case Item::VenomDagger:
    return 50;
  case Item::StoneSigil:
    return 30;
  case Item::MartyrWraps:
    return 45;
  case Item::AgnosticCollar:
    return 50;
  case Item::MagePlate:
    return 40;
  case Item::BlueBead:
    return 10;
  case Item::VampiricBlade:
    return 50;
  case Item::ViperWard:
    return 65;
  case Item::SoulOrb:
    return 65;

  case Item::KegOfHealth:
    return 70;
  case Item::KegOfMana:
    return 70;
  case Item::ElvenBoots:
    return 50;
  case Item::DwarvenGauntlets:
    return 50;
  case Item::AmuletOfYendor:
    return 100;
  case Item::OrbOfZot:
    return 100;
  case Item::AlchemistScroll:
    return 40;
  case Item::WickedGuitar:
    return 11;

  case Item::BearMace:
    return 35;
  case Item::PerseveranceBadge:
    return 20;
  case Item::ReallyBigSword:
    return 35;
  case Item::Shield:
    return 35;
  case Item::SlayerWand:
    return 10;
  case Item::Sword:
    return 35;

  case Item::FreeHealthPotion:
  case Item::FreeManaPotion:
  case Item::HealthPotion:
  case Item::ManaPotion:
  case Item::FortitudeTonic:
  case Item::BurnSalve:
  case Item::StrengthPotion:
  case Item::Schadenfreude:
  case Item::QuicksilverPotion:
  case Item::ReflexPotion:
  case Item::CanOfWhupaz:
    return 10;

  case Item::CompressionSeal:
  case Item::TransmutationSeal:
  case Item::TranslocationSeal:
    return 5;

  case Item::ShopScroll:
    return 20;
  case Item::PatchesTheTeddy:
    return 10;

  case Item::Gorgward:
    return 50;

  case Item::FabulousTreasure:
    return 1;
  case Item::DragonShield:
    return 100;
  case Item::NamtarsWard:
    return 100;
  case Item::AvatarsCodex:
    return 50;
  case Item::NagaCauldron:
    return 35;
  case Item::SensationStone:
    return 150;

  case Item::PrayerBead:
    return -1;
  case Item::EnchantedPrayerBead:
    return -1;

  case Item::Skullpicker:
  case Item::Wereward:
  case Item::Gloat:
  case Item::Will:
    return 60;

  case Item::TikkisCharm:
    return 5;

  case Item::AnyPotion:
    return 0;
  }
}

constexpr bool isPotion(Item item)
{
  return item == Item::FreeHealthPotion || item == Item::FreeManaPotion || item == Item::HealthPotion ||
         item == Item::ManaPotion || item == Item::FortitudeTonic || item == Item::BurnSalve ||
         item == Item::StrengthPotion || item == Item::Schadenfreude || item == Item::QuicksilverPotion ||
         item == Item::ReflexPotion || item == Item::CanOfWhupaz;
}

constexpr bool isSmall(Item item)
{
  return isPotion(item) || item == Item::Spoon || item == Item::DragonSoul || item == Item::WitchalokPendant ||
         item == Item::BalancedDagger || item == Item::VenomDagger || item == Item::StoneSigil ||
         item == Item::BlueBead || item == Item::ViperWard || item == Item::SoulOrb || item == Item::CompressionSeal ||
         item == Item::TransmutationSeal || item == Item::TranslocationSeal || item == Item::PerseveranceBadge ||
         item == Item::Gorgward || item == Item::PrayerBead || item == Item::EnchantedPrayerBead ||
         item == Item::TikkisCharm;
}

constexpr bool canGroup(Item item)
{
  return isPotion(item);
}
