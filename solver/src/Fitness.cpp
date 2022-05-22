#include "solver/Fitness.hpp"

#include <numeric>
#include <sstream>

namespace fitness1
{
  unsigned computeInventoryScore(const Hero& hero)
  {
    const auto& itemsAndSpells = hero.getItemsAndSpells();
    return std::accumulate(begin(itemsAndSpells), end(itemsAndSpells), 0u, [](const unsigned sum, const auto& entry) {
      if (const auto item = std::get_if<Item>(&entry.itemOrSpell))
        return sum + static_cast<unsigned>(entry.price + 2 * entry.conversionPoints);
      else
        return sum + 300;
    }) / 2u;
  }

  unsigned computeResourceScore(const SimpleResources& resources)
  {
    return clampedTo<unsigned>((2u * resources.numHiddenTiles + resources.numWalls) / 10u +
                               resources.spells.size() * 100u + resources.freeSpells.size() * 90u);
  }

  unsigned computeHeroScore(const Hero& hero)
  {
    return 50u * hero.getLevel() + 50u * hero.getXP() / hero.getXPforNextLevel() +
           10u * hero.getDamageVersusStandard() + 3u * hero.getHitPoints() + 5u * hero.getManaPoints() +
           hero.getPiety();
  }

  unsigned computeMonsterScore(const Monsters& monsters)
  {
    return std::accumulate(begin(monsters), end(monsters), 0u, [](const unsigned sum, const Monster& monster) {
      return sum + monster.getHitPoints() * 100u + monster.getPhysicalResistPercent() +
             monster.getMagicalResistPercent();
    });
  }
} // namespace fitness1

int StateFitnessRating1::operator()(const GameState& state) const
{
  if (state.hero.isDefeated())
    return GAME_LOST;
  if (state.visibleMonsters.empty())
    return GAME_WON;
  using namespace fitness1;
  return static_cast<int>(computeHeroScore(state.hero) + computeInventoryScore(state.hero) +
                          computeResourceScore(state.resources)) -
         static_cast<int>(computeMonsterScore(state.visibleMonsters));
}

std::string StateFitnessRating1::explain(const GameState& state) const
{
  using namespace fitness1;
  if (state.visibleMonsters.empty())
    return "No monsters remaining, score = " + std::to_string(GAME_WON);
  const auto& hero = state.hero;
  const auto heroScore = computeHeroScore(hero);
  const auto damage = hero.getDamageVersusStandard();
  const auto inventoryScore = computeInventoryScore(state.hero);
  const auto resourceScore = computeResourceScore(state.resources);
  const auto monsterScore = computeMonsterScore(state.visibleMonsters);
  std::stringstream sstr;
  sstr << "Hero (total): " << heroScore << std::endl
       << "  Level = " << hero.getLevel() << " -> " << 50 * hero.getLevel() << std::endl
       << "  XP = " << hero.getXP() << " -> " << 50 * hero.getXP() / hero.getXPforNextLevel() << std::endl
       << "  Damage = " << damage << " -> " << 10 * damage << std::endl
       << "  HP = " << hero.getHitPoints() << " -> " << 3 * hero.getHitPoints() << std::endl
       << "  MP = " << hero.getManaPoints() << " -> " << 5 * hero.getManaPoints() << std::endl
       << "  Piety = " << hero.getPiety() << std::endl
       << "Inventory: " << inventoryScore << std::endl
       << "Resources: " << resourceScore << std::endl
       << "Monsters: " << monsterScore << std::endl
       << "Total score: "
       << static_cast<int>(heroScore + inventoryScore + resourceScore) - static_cast<int>(monsterScore) << std::endl;
  return sstr.str();
}

int StateFitnessRating2::operator()(const GameState& state) const
{
  if (state.hero.isDefeated())
    return GAME_LOST;
  const auto& monsters = state.visibleMonsters;
  const auto numMonsters = monsters.size();
  if (numMonsters == 0)
    return GAME_WON;
  if (state.activeMonster >= numMonsters)
  {
    assert(false);
    return GAME_LOST;
  }
  const auto& hero = state.hero;
  const auto& monster = monsters[state.activeMonster];
  const auto numHeroGetsHit = ceil(static_cast<double>(monster.getHitPoints()) / hero.getDamageOutputVersus(monster)) +
                              (hero.hasInitiativeVersus(monster) ? -1 : 0);
  const auto expectedHPLoss = numHeroGetsHit * hero.predictDamageTaken(monster.getDamage(), monster.damageType());
  const auto awards = hero.has(HeroStatus::DeathProtection) ? 100 : 0;
  const auto penalties = (hero.has(HeroStatus::FirstStrikeTemporary) && numHeroGetsHit > 0 ? 50 : 0);
  return static_cast<int>(hero.getHitPoints() - expectedHPLoss - 1000 * (numMonsters - 1) + awards - penalties);
}
