#pragma once

#include "engine/GodsAndBoons.hpp"
#include "engine/GlowingGuardian.hpp"
#include "engine/Items.hpp"
#include "engine/Jehora.hpp"
#include "engine/Monster.hpp"
#include "engine/Resources.hpp"

#include <map>
#include <optional>
#include <variant>
#include <vector>

class Hero;

enum class Spell;

struct JehoraTriggered
{
};

struct [[nodiscard]] PietyChange
{
public:
  PietyChange(int deltaPoints = 0);
  PietyChange(Pact activated);
  PietyChange(JehoraTriggered);
  const std::vector<int>& operator()() const;
  std::optional<Pact> activatedPact() const;
  bool randomJehoraEvent() const;
  PietyChange& operator+=(const PietyChange&);
  PietyChange& operator+=(int deltaPoints);

private:
  std::vector<int> values;
  std::optional<Pact> pact;
  bool jehora;
};

using ItemOrSpell = std::variant<Item, Spell>;

class Faith
{
public:
  bool followDeity(God god, Hero& hero, int numRevealedTiles);
  std::optional<God> getFollowedDeity() const;
  bool canFollow(God god, const Hero& hero) const;

  int getPiety() const;
  int getMaxPiety() const;
  bool has(Boon) const;
  int boonCount(Boon) const;
  int getIndulgence() const;

  void gainPiety(int pointsGained);
  void losePiety(int pointsLost, Hero& hero, Monsters& allMonsters);

  // Apply one of Jehora's random event (piety gain or punishment)
  void applyRandomJehoraEvent(Hero& hero);

  void apply(PietyChange, Hero& hero, Monsters& allMonsters);

  int isAvailable(Boon boon, const Hero& hero, const Monsters& allMonsters, const Resources& resources) const;
  bool request(Boon boon, Hero& hero, Monsters& allMonstersOnFloor, Resources& resources);
  int getCosts(Boon boon, const Hero& hero) const;

  std::optional<Pact> getPact() const;
  bool enter(Pact pact);
  bool enteredConsensus() const;

  void desecrate(God altar, Hero& hero, Monsters& allMonsters, bool hasAgnosticCollar);

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

  PietyChange drankPotion(Potion potion);
  PietyChange lifeStolen(const Monster& monster);
  PietyChange bloodPoolConsumed(int numBloodTithe);
  PietyChange becamePoisoned();
  PietyChange becameManaBurned();
  PietyChange manaPointsBurned(int pointsLost);

  PietyChange converted(ItemOrSpell itemOrSpell, bool wasSmall);

  // specific to Earthmother
  PietyChange plantDestroyed();

  // specific to Jehora
  PietyChange monsterRevealed();

  // specific to Tikki Tooki
  PietyChange receivedHit(const Monster& monster);
  PietyChange dodgedAttack();
  PietyChange deathProtectionTriggered();

  // Instantly triggers special Taurog punishment
  void convertedTaurogItem(Hero& hero, Monsters& allMonsters);

private:
  void initialBoon(God god, Hero& hero, int numRevealedTiles);
  void punish(God god, Hero& hero, Monsters& allMonsters);

  std::optional<God> followedDeity;
  std::vector<Boon> boons;
  int piety;
  int indulgence;
  int numDesecrated;
  std::optional<Pact> pact;
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

  GlowingGuardian glowingGuardian;
  Jehora jehora;
};
