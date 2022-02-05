#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <variant>

struct ItemProperties
{
  std::string_view name;
  int price;
  int conversionPoints;
  bool isSmall;
};

enum class BlacksmithItem
{
  PerseveranceBadge,
  SlayerWand,
  ReallyBigSword,
  BearMace,
  Sword,
  Shield,
  Last = Shield
};

enum class Potion
{
  HealthPotion,
  ManaPotion,
  FortitudeTonic,
  BurnSalve,
  StrengthPotion,
  Schadenfreude,
  QuicksilverPotion,
  ReflexPotion,
  CanOfWhupaz,
  LastInShop = CanOfWhupaz,
  CourageJuice, // Available only in very few dungeons
  CourageJuiceCorroded, // Workaround: Often dropped by corrosive monsters
  Last = CourageJuiceCorroded
};

enum class AlchemistSeal
{
  CompressionSeal,
  TransmutationSeal,
  TranslocationSeal,
  Last = TranslocationSeal
};

enum class ShopItem
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
  Last = WickedGuitar
};

enum class BossReward
{
  FabulousTreasure,
  DragonShield,
  NamtarsWard,
  AvatarsCodex,
  NagaCauldron,
  SensationStone,
  Last = SensationStone,
};

enum class MiscItem
{
  // Misc Items
  ShopScroll,
  PatchesTheTeddy,
  Food,
  FoodStack,
  InfiniteManaPotions, // 99 mana potions with 0 CP (True Grit puzzle level)
  // Subdungeon Items, Lockerable
  Gorgward,
  // God Items
  PrayerBead,
  EnchantedPrayerBead,
  // Puzzle Items
  TikkisCharm,
  // Items available only in specific dungeon(s)
  WispGem,
  WallCruncher,
  Charm, // MonsterMachine: small item; +1 HP; + 1DAM
  // Workaround: These items are often dropped by corrosive monsters, stepping on tile will corrode.
  WispGemCorroded,
  WallCruncherCorroded,
  CharmCorroded,
  Last = Charm
};

enum class TaurogItem
{
  Skullpicker,
  Wereward,
  Gloat,
  Will
};

template <typename Key, typename Value, std::size_t Size>
struct Map
{
  std::array<std::pair<Key, Value>, Size> data;

  [[nodiscard]] constexpr Value at(const Key& key) const
  {
    const auto iter = std::find_if(begin(data), end(data), [&key](const auto& v) { return v.first == key; });
    if (iter != end(data))
      return iter->second;
    else
      throw std::range_error("Not found");
  }
};

using namespace std::string_view_literals;
using LockerableItem = std::variant<ShopItem, BossReward>;
using Item = std::variant<BlacksmithItem, Potion, AlchemistSeal, ShopItem, BossReward, MiscItem, TaurogItem>;

static constexpr Map<Item, ItemProperties, 82> items{
    std::make_pair(BlacksmithItem::PerseveranceBadge, ItemProperties{"Perseverance Badge"sv, 15, 20, true}),
    {BlacksmithItem::SlayerWand, {"Slayer Wand"sv, 5, 10, false}},
    {BlacksmithItem::ReallyBigSword, {"Really Big Sword"sv, 12, 35, false}},
    {BlacksmithItem::BearMace, {"Bear Mace"sv, 12, 35, false}},
    {BlacksmithItem::Sword, {"Sword"sv, 25, 35, false}},
    {BlacksmithItem::Shield, {"Shield"sv, 15, 35, false}},
    {Potion::HealthPotion, {"Health Potion"sv, 10, 10, true}},
    {Potion::ManaPotion, {"Mana Potion"sv, 10, 10, true}},
    {Potion::FortitudeTonic, {"Fortitude Tonic"sv, 8, 10, true}},
    {Potion::BurnSalve, {"Burn Salve"sv, 8, 10, true}},
    {Potion::StrengthPotion, {"Strength Potion"sv, 15, 10, true}},
    {Potion::Schadenfreude, {"Schadenfreude"sv, 15, 10, true}},
    {Potion::QuicksilverPotion, {"Quicksilver Potion"sv, 15, 10, true}},
    {Potion::ReflexPotion, {"Reflex Potion"sv, 15, 10, true}},
    {Potion::CanOfWhupaz, {"Can of Whupaz"sv, 20, 10, true}},
    {Potion::CourageJuice, {"\"Courage Juice\""sv, 15, 10, true}},
    {Potion::CourageJuiceCorroded, {"\"Courage Juice\" (corroded tile)"sv, 15, 10, true}},
    {AlchemistSeal::CompressionSeal, {"Compression Seal"sv, 16, 5, true}},
    {AlchemistSeal::TransmutationSeal, {"Transmutation Seal"sv, 36, 5, true}},
    {AlchemistSeal::TranslocationSeal, {"Translocation Seal"sv, 43, 5, true}},
    {ShopItem::BadgeOfHonour, {"Badge of Honour"sv, 18, 40, false}},
    {ShopItem::BloodySigil, {"Bloody Sigil"sv, 8, 45, false}},
    {ShopItem::FineSword, {"Fine Sword"sv, 15, 35, false}},
    {ShopItem::PendantOfHealth, {"Pendant of Health"sv, 15, 35, false}},
    {ShopItem::PendantOfMana, {"Pendant of Mana"sv, 12, 35, false}},
    {ShopItem::Spoon, {"Spoon"sv, 1, 1, true}},
    {ShopItem::TowerShield, {"Tower Shield"sv, 14, 35, false}},
    {ShopItem::TrollHeart, {"Troll Heart"sv, 16, 55, false}},
    {ShopItem::PiercingWand, {"Piercing Wand"sv, 13, 30, false}},
    {ShopItem::RockHeart, {"Rock Heart"sv, 14, 60, false}},
    {ShopItem::DragonSoul, {"Dragon Soul"sv, 23, 50, true}},
    {ShopItem::FireHeart, {"Fire Heart"sv, 20, 12, false}},
    {ShopItem::CrystalBall, {"Crystal Ball"sv, 15, 50, false}},
    {ShopItem::WitchalokPendant, {"Witchalok Pendant"sv, 19, 30, true}},
    {ShopItem::BattlemageRing, {"Battlemage Ring"sv, 25, 20, false}},
    {ShopItem::HerosHelm, {"Hero's Helm"sv, 18, 50, false}},
    {ShopItem::Platemail, {"Platemail"sv, 23, 40, false}},
    {ShopItem::Whurrgarbl, {"Whurrgarbl"sv, 15, 45, false}},
    {ShopItem::Trisword, {"Trisword"sv, 12, 35, false}},
    {ShopItem::BalancedDagger, {"Balanced Dagger"sv, 12, 44, true}},
    {ShopItem::GlovesOfMidas, {"Gloves Of Midas"sv, 10, 45, false}},
    {ShopItem::VenomDagger, {"Venom Dagger"sv, 16, 50, true}},
    {ShopItem::StoneSigil, {"Stone Sigil"sv, 14, 30, true}},
    {ShopItem::MartyrWraps, {"Martyr Wraps"sv, 15, 45, false}},
    {ShopItem::AgnosticCollar, {"Agnostic Collar"sv, 25, 50, false}},
    {ShopItem::MagePlate, {"Mage Plate"sv, 20, 40, false}},
    {ShopItem::BlueBead, {"Blue Bead"sv, 5, 10, true}},
    {ShopItem::VampiricBlade, {"Vampiric Blade"sv, 25, 50, false}},
    {ShopItem::ViperWard, {"Viper Ward"sv, 16, 65, true}},
    {ShopItem::SoulOrb, {"Soul Orb"sv, 16, 65, true}},
    {ShopItem::KegOfHealth, {"Keg Of Health"sv, 25, 70, false}},
    {ShopItem::KegOfMana, {"Keg Of Mana"sv, 25, 70, false}},
    {ShopItem::ElvenBoots, {"Elven Boots"sv, 35, 50, false}},
    {ShopItem::DwarvenGauntlets, {"Dwarven Gauntlets"sv, 35, 50, false}},
    {ShopItem::AmuletOfYendor, {"Amulet of Yendor"sv, 45, 100, false}},
    {ShopItem::OrbOfZot, {"Orb of Zot"sv, 45, 100, false}},
    {ShopItem::AlchemistScroll, {"Alchemist Scroll"sv, 13, 40, false}},
    {ShopItem::WickedGuitar, {"Wicked Guitar"sv, 11, 11, false}},
    {BossReward::FabulousTreasure, {"Fabulous Treasure"sv, 95, 1, false}},
    {BossReward::DragonShield, {"Dragon Shield"sv, 23, 100, false}},
    {BossReward::NamtarsWard, {"Namtar's Ward"sv, 50, 100, false}},
    {BossReward::AvatarsCodex, {"Avatar's Codex"sv, 35, 50, false}},
    {BossReward::NagaCauldron, {"Naga Cauldron"sv, 12, 35, false}},
    {BossReward::SensationStone, {"Sensation Stone"sv, 25, 150, false}},
    {MiscItem::ShopScroll, {"Shop Scroll"sv, 10, 20, false}},
    {MiscItem::PatchesTheTeddy, {"Patches the Teddy"sv, 0, 10, false}},
    {MiscItem::Food, {"Food"sv, 0, 3, true}},
    {MiscItem::FoodStack, {"Food Stack"sv, 0, 27, true}},
    {MiscItem::Gorgward, {"Gorgward"sv, 18, 50, true}},
    {MiscItem::PrayerBead, {"Prayer Bead"sv, -1, -1, true}},
    {MiscItem::EnchantedPrayerBead, {"Enchanted Prayer Bead"sv, -1, -1, true}},
    {MiscItem::TikkisCharm, {"Tikki's Charm"sv, 1, 5, true}},
    {MiscItem::WispGem, {"Wisp Gem"sv, 1, 10, true}},
    {MiscItem::WallCruncher, {"Wall Cruncher"sv, 1, 25, true}},
    {MiscItem::Charm, {"Charm"sv, 1, 1, true}},
    {MiscItem::WispGemCorroded, {"Wisp Gem (corroded tile)"sv, 1, 10, true}},
    {MiscItem::WallCruncherCorroded, {"Wall Cruncher (corroded tile)"sv, 1, 25, true}},
    {MiscItem::CharmCorroded, {"Charm (corroded tile)"sv, 1, 1, true}},
    {TaurogItem::Skullpicker, {"Skullpicker"sv, 0, 60, false}},
    {TaurogItem::Wereward, {"Wereward"sv, 0, 60, false}},
    {TaurogItem::Gloat, {"Gloat"sv, 0, 60, false}},
    {TaurogItem::Will, {"Will"sv, 0, 60, false}},
};

constexpr const char* toString(Item item)
{
  return items.at(item).name.data();
}

template <class ItemSubtype>
constexpr const char* toString(ItemSubtype item)
{
  return toString(Item{item});
}

constexpr int price(Item item)
{
  return items.at(item).price;
}

constexpr int initialConversionPoints(Item item)
{
  return items.at(item).conversionPoints;
}

constexpr int isSmall(Item item)
{
  return items.at(item).isSmall;
}
