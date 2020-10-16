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
  TikkiTooki
};

enum class Boon
{
  // Dracul
  BloodCurse,
  // Glowing Guardian
  Humility,
  // Jehora
  Petition,
  // Mystera Annur
  Flames,
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
  case Boon::BloodCurse:
    return "Blood Curse";
  case Boon::Flames:
    return "Flames";
  case Boon::Humility:
    return "Humility";
  case Boon::Petition:
    return "Petition";
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
  bool hasBoon(Boon) const;

  void gainPiety(int pointsGained);
  void losePiety(int pointsLost, Hero& hero);

  // Apply one of Jehora's random event (piety gain or punishment)
  void applyRandomJehoraEvent(Hero& hero);

  void apply(PietyChange, Hero& hero);

  // bool request(Boon boon, Hero& hero);
  // int getCosts(Boon boon, const Hero& hero) const;
  // int isAvailable(Boon boon, const Hero& hero) const;

  void desecrate(God altar, Hero& hero);

  // The following methods address the gods' likes and dislikes.
  PietyChange monsterKilled(const Monster& monster, int heroLevel, bool monsterWasBurning);
  PietyChange monsterPoisoned(const Monster& monster);
  // Regular spells (no target or targeting monster), including Imawal
  PietyChange spellCast(Spell spell, int manaCost);
  // Imawal cast on empty space
  PietyChange imawalCreateWall(int manaCost);
  // Imawal cast on a plant
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

  // specific to Binlor
  PietyChange wallDestroyed();
  PietyChange wallCreated();

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
