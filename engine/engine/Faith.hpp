#pragma once

#include "engine/GlowingGuardian.hpp"
#include "engine/GodsAndBoons.hpp"
#include "engine/Items.hpp"
#include "engine/Jehora.hpp"
#include "engine/Monster.hpp"
#include "engine/PietyChange.hpp"
#include "engine/Resources.hpp"

#include <map>
#include <optional>
#include <variant>
#include <vector>

class Hero;

enum class Spell;

using ItemOrSpell = std::variant<Item, Spell>;

class Faith
{
public:
  explicit Faith(std::optional<GodOrPactmaker> preparedAltar = {});

  bool followDeity(God god, Hero& hero, unsigned numRevealedTiles);
  std::optional<God> getFollowedDeity() const;
  bool canFollow(God god, const Hero& hero) const;

  unsigned getPiety() const;
  unsigned getMaxPiety() const;
  bool has(Boon) const;
  unsigned boonCount(Boon) const;
  unsigned getIndulgence() const;

  void gainPiety(unsigned pointsGained);
  void losePiety(unsigned pointsLost, Hero& hero, Monsters& allMonsters);

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
  PietyChange monsterKilled(const Monster& monster, unsigned heroLevel, bool monsterWasBurning);
  PietyChange monsterPoisoned(const Monster& monster);
  // Regular spells (no target or targeting monster), including Imawal cast on monster
  PietyChange spellCast(Spell spell, unsigned manaCost);
  // Imawal cast on empty space (Binlor)
  PietyChange imawalCreateWall(unsigned manaCost);
  // Imawal cast on a plant (Earthmother)
  PietyChange imawalPetrifyPlant(unsigned manaCost);
  PietyChange levelGained();

  PietyChange drankPotion(Potion potion);
  PietyChange lifeStolen(const Monster& monster);
  PietyChange bloodPoolConsumed(unsigned numBloodTithe);
  PietyChange becamePoisoned();
  PietyChange becameManaBurned();
  PietyChange manaPointsBurned(unsigned pointsLost);

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

  bool preparationPenaltyApplies() const { return preparedAltar == followedDeity; }

private:
  void initialBoon(God god, Hero& hero, unsigned numRevealedTiles);
  void punish(God god, Hero& hero, Monsters& allMonsters);

  std::optional<God> followedDeity;
  std::optional<God> preparedAltar;
  std::optional<Pact> pact;
  std::vector<Boon> boons;
  unsigned piety{0};
  unsigned indulgence{0};
  unsigned numDesecrated{0};
  bool consensus{false};

  // counts that may affect awarded piety
  unsigned numMonstersKilled{0};
  unsigned numSpellsCast{0};
  unsigned numManaPointsSpent{0};
  unsigned numConsecutiveLevelUpsWithGlowingGuardian{0};

  struct MonsterPietyHistory
  {
    // for Dracul
    bool hadLifeStolen{false};
    // for Tikki Tooki
    bool hitHero{false};
    bool becamePoisoned{false};
  };
  std::map<int, MonsterPietyHistory> history;

  GlowingGuardian glowingGuardian;
  Jehora jehora;
};
