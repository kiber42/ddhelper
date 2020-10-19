#pragma once

#include "Jehora.hpp"
#include "Monster.hpp"

#include <map>
#include <optional>
#include <vector>

enum class God
{
  BinlorIronshield,
  Dracul,
  TheEarthmother,
  GlowingGuardian,
  JehoraJeheyu,
  MysteraAnnur,
  Taurog,
  TikkiTooki,
  Last = TikkiTooki
};

enum class Boon
{
  // Binlor Ironshield
  StoneSoup,
  StoneSkin,
  StoneForm,
  StoneFist,
  StoneHeart,

  // Dracul
  BloodCurse,
  BloodTithe,
  BloodHunger,
  BloodShield,
  BloodSwell,

  // The Earthmother
  Plantation,
  Clearance,
  Greenblood,
  Entanglement,
  VineForm,

  // Glowing Guardian
  Humility,
  Absolution,
  Cleansing,
  Protection,
  Enlightenment,

  // Jehora
  Petition,
  // Mystera Annur
  Flames,
  Last = Flames
};

constexpr const char* toString(God god)
{
  switch (god)
  {
  case God::BinlorIronshield:
    return "Binlor Ironshield";
  case God::Dracul:
    return "Dracul";
  case God::TheEarthmother:
    return "The Earthmother";
  case God::GlowingGuardian:
    return "Glowing Guardian";
  case God::JehoraJeheyu:
    return "Jehora Jeheyu";
  case God::MysteraAnnur:
    return "Mystera Annur";
  case God::Taurog:
    return "Taurog";
  case God::TikkiTooki:
    return "Tikki Tooki";
  }
}

constexpr const char* toString(Boon boon)
{
  switch (boon)
  {
  case Boon::StoneSoup:
    return "Stone Soup";
  case Boon::StoneSkin:
    return "Stone Skin";
  case Boon::StoneForm:
    return "Stone Form";
  case Boon::StoneFist:
    return "Stone Fist";
  case Boon::StoneHeart:
    return "Stone Heart";

  case Boon::BloodCurse:
    return "Blood Curse";
  case Boon::BloodTithe:
    return "Blood Tithe";
  case Boon::BloodHunger:
    return "Blood Hunger";
  case Boon::BloodShield:
    return "Blood Shield";
  case Boon::BloodSwell:
    return "Blood Swell";

  case Boon::Plantation:
    return "Plantation";
  case Boon::Clearance:
    return "Clearance";
  case Boon::Greenblood:
    return "Greenblood";
  case Boon::Entanglement:
    return "Entanglement";
  case Boon::VineForm:
    return "Vine Form";

  case Boon::Humility:
    return "Humility";
  case Boon::Absolution:
    return "Absolution";
  case Boon::Cleansing:
    return "Cleansing";
  case Boon::Protection:
    return "Protection";
  case Boon::Enlightenment:
    return "Enlightenment";

  case Boon::Flames:
    return "Flames";
  case Boon::Petition:
    return "Petition";
  }
}

constexpr bool allowRepeatedUse(Boon boon)
{
  return boon == Boon::StoneSkin || boon == Boon::StoneHeart || boon == Boon::BloodTithe || boon == Boon::BloodHunger ||
         boon == Boon::BloodSwell || boon == Boon::Plantation || boon == Boon::Clearance || boon == Boon::Greenblood ||
         boon == Boon::Entanglement || boon == Boon::VineForm || boon == Boon::Absolution || boon == Boon::Cleansing ||
         boon == Boon::Protection || boon == Boon::Absolution || boon == Boon::Cleansing || boon == Boon::Protection;
}

constexpr God deity(Boon boon)
{
  switch (boon)
  {
  case Boon::StoneSoup:
  case Boon::StoneSkin:
  case Boon::StoneForm:
  case Boon::StoneFist:
  case Boon::StoneHeart:
    return God::BinlorIronshield;

  case Boon::BloodCurse:
  case Boon::BloodTithe:
  case Boon::BloodHunger:
  case Boon::BloodShield:
  case Boon::BloodSwell:
    return God::Dracul;

  case Boon::Plantation:
  case Boon::Clearance:
  case Boon::Greenblood:
  case Boon::Entanglement:
  case Boon::VineForm:
    return God::TheEarthmother;

  case Boon::Humility:
  case Boon::Absolution:
  case Boon::Cleansing:
  case Boon::Protection:
  case Boon::Enlightenment:
    return God::GlowingGuardian;

  case Boon::Flames:
    return God::MysteraAnnur;
  case Boon::Petition:
    return God::JehoraJeheyu;
  }
}

// TODO: Add pacts

class Hero;
enum class Item;
enum class Spell;

struct JehoraTriggered
{
};

struct [[nodiscard]] PietyChange
{
public:
  PietyChange(int deltaPoints = 0);
  PietyChange(JehoraTriggered);
  int operator()() const;
  bool randomJehoraEvent() const;
  PietyChange& operator+=(const PietyChange&);

private:
  std::optional<int> value;
};

class Faith
{
public:
  bool followDeity(God god, Hero& hero);
  std::optional<God> getFollowedDeity() const;

  int getPiety() const;
  int getMaxPiety() const;
  int boonCount(Boon) const;

  void gainPiety(int pointsGained);
  void losePiety(int pointsLost, Hero& hero);

  // Apply one of Jehora's random event (piety gain or punishment)
  void applyRandomJehoraEvent(Hero& hero);

  void apply(PietyChange, Hero& hero);

  bool request(Boon boon, Hero& hero);
  int getCosts(Boon boon, const Hero& hero) const;
  int isAvailable(Boon boon, const Hero& hero) const;

  void desecrate(God altar, Hero& hero);

  // The following methods address the gods' likes and dislikes.
  PietyChange monsterKilled(const Monster& monster, int heroLevel, bool monsterWasBurning);
  PietyChange monsterPoisoned(const Monster& monster);
  // Regular spells (no target or targeting monster), including Imawal cast on monster
  PietyChange spellCast(Spell spell, int manaCost);
  // Imawal cast on empty space (Binlor)
  PietyChange imawalCreateWall(int manaCost);
  // Imawal cast on a plant (Earthmother)
  PietyChange imawalPetrifyPlant(int manaCost);
  PietyChange levelGained();

  PietyChange itemUsed(Item item);
  PietyChange lifeStolen(const Monster& monster);
  PietyChange bloodPoolConsumed(int numBloodTithe);
  PietyChange becamePoisoned();
  PietyChange becameManaBurned();
  PietyChange manaPointsBurned(int pointsLost);

  PietyChange converted(Item item);
  PietyChange converted(Spell spell);

  // specific to Earthmother
  PietyChange plantDestroyed();

  // specific to Jehora
  PietyChange monsterRevealed();

  // specific to Tikki Tooki
  PietyChange receivedHit(const Monster& monster);
  PietyChange dodgedAttack();
  PietyChange deathProtectionTriggered();

private:
  void initialBoon(God god, Hero& hero);
  void punish(God god, Hero& hero);

  std::optional<God> followedDeity;
  std::vector<Boon> boons;
  int piety;
  int indulgence;
  int numDesecrated;
  bool pactMade;
  bool consensus;

  // counts that may affect awarded piety
  int numMonstersKilled;
  int numSpellsCast;
  int numManaPointsSpent;
  int numConsecutiveLevelUpsWithGlowingGuardian;

  struct MonsterPietyHistory
  {
    // for Dracul
    bool hadLifeStolen;
    // for Tikki Tooki
    bool hitHero;
    bool becamePoisoned;
  };
  std::map<int, MonsterPietyHistory> history;

  Jehora jehora;
};
