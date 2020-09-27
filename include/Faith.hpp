#pragma once

#include "Monster.hpp"

#include <map>
#include <optional>
#include <random>
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
enum class Spell;

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

  // Apply one of Jehora's random punishments
  void applyRandomPunishment(Hero& hero);

  // Award or remove piety points (deltaPoints >0 or <0, respectively).
  // If points drop below 0, the corresponding punishment is applied.
  // Use deltaPoints = 0 to apply a random punishment by Jehora.
  // If deltaPoints is nullopt, do nothing.
  void updatePiety(std::optional<int> deltaPoints, Hero& hero);

  // bool request(Boon boon, Hero& hero);
  // int getCosts(Boon boon, const Hero& hero) const;
  // int isAvailable(Boon boon, const Hero& hero) const;

  void desecrate(God altar, Hero& hero);

  // The following methods address the gods' likes and dislikes.
  // The optional return value indicates piety awarded or removed.
  // A non-nullopt return value of 0 indicates a random punishment by Jehora.
  // See the updatePiety helper method.
  std::optional<int> monsterKilled(const Monster& monster, int heroLevel, bool monsterWasBurning);
  std::optional<int> monsterPoisoned(const Monster& monster);
  // Regular spells (no target or targeting monster), including Imawal
  std::optional<int> spellCast(Spell spell, int manaCost);
  // Imawal cast on empty space
  std::optional<int> imawalCreateWall(int manaCost);
  // Imawal cast on a plant
  std::optional<int> imawalPetrifyPlant(int manaCost);
  std::optional<int> levelGained();

  std::optional<int> potionConsumed(/* Potion potion */);
  std::optional<int> lifeStolen(const Monster& monster);
  std::optional<int> becamePoisoned();
  std::optional<int> manaBurned();

  // TODO std::optional<int> converted(Item item);
  std::optional<int> converted(/* Potion potion */);
  std::optional<int> converted(Spell spell);

  // specific to Binlor
  std::optional<int> wallDestroyed();
  std::optional<int> wallCreated();

  // specific to Dracul
  std::optional<int> bloodPoolConsumed();

  // specific to Earthmother
  std::optional<int> plantDestroyed();

  // specific to Jehora
  std::optional<int> monsterRevealed();

  // specific to Tikki Tooki
  std::optional<int> receivedHit(const Monster& monster);
  std::optional<int> dodgedAttack();
  std::optional<int> deathProtectionTriggered();

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

  struct PietyEvents
  {
    // for Dracul
    bool hadLifeStolen;
    // for Tikki Tooki
    bool hitHero;
    bool becamePoisoned;
  };
  std::map<int, PietyEvents> pietyEvents;

  class Jehora
  {
  public:
    Jehora();
    int initialPietyBonus(int heroLevel);
    int operator()();
    void applyRandomPunishment(Hero& hero);

  private:
    std::mt19937 generator;
    int happiness;
    int thresholdPoison;
    int thresholdManaBurn;
    int thresholdHealthLoss;
    int thresholdWeakened;
    int thresholdCorrosion;
    int thresholdCursed;
  };
  Jehora jehora;
};
