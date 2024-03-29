#include "solver/SolverTools.hpp"

#include "engine/Combat.hpp"
#include "engine/Magic.hpp"

#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <variant>

namespace solver
{
  thread_local std::mt19937 generator{std::random_device{"/dev/urandom"}()};

  Step generateValidStep(const GameState& state, int stepTypeIndex)
  {
    auto otherAltar = [&hero = state.hero, altars = state.resources.altars]() mutable -> std::optional<God> {
      altars.erase(std::remove(begin(altars), end(altars), GodOrPactmaker{Pactmaker::ThePactmaker}), end(altars));
      if (altars.empty())
        return {};
      std::shuffle(begin(altars), end(altars), generator);
      if (!hero.getFollowedDeity())
        return std::get<God>(altars.front());
      auto altarIt =
          std::find_if(begin(altars), end(altars), [followedDeity = *hero.getFollowedDeity()](const auto altar) {
            return altar != GodOrPactmaker{followedDeity};
          });
      if (altarIt != end(altars))
        return std::get<God>(*altarIt);
      return {};
    };

    auto randomElement = [](const auto& vec) {
      assert(!vec.empty());
      const auto index = std::uniform_int_distribution<std::size_t>(0, vec.size() - 1u)(generator);
      return vec[index];
    };

    switch (stepTypeIndex)
    {
    case 0:
      assert(state.activeMonster < state.visibleMonsters.size());
      return Attack{};
    case 1:
    {
      assert(state.activeMonster < state.visibleMonsters.size());
      auto spellCounts = state.hero.getSpellCounts();
      std::shuffle(begin(spellCounts), end(spellCounts), generator);
      const auto spellIt =
          std::find_if(begin(spellCounts), end(spellCounts),
                       [&state, &monster = state.visibleMonsters[state.activeMonster]](const auto& spellCount) {
                         const Spell spell = spellCount.first;
                         return Magic::isPossible(state.hero, monster, spell, state.resources);
                       });
      if (spellIt != end(spellCounts))
        return Cast{spellIt->first};
      break;
    }
    case 2:
      if (state.resources.numHiddenTiles > 0)
        return Uncover{1};
      break;
    case 3:
    {
      if (!state.hero.hasRoomFor(Potion::HealthPotion) || state.hero.gold() == 0)
        break;
      auto shops = state.resources.shops;
      std::shuffle(begin(shops), end(shops), generator);
      const auto shopIt = std::find_if(begin(shops), end(shops), [&hero = state.hero](const Item item) {
        return hero.hasRoomFor(item) && hero.canAfford(item);
      });
      if (shopIt != end(shops))
        return Buy{*shopIt};
      break;
    }
    case 4:
    {
      auto itemCounts = state.hero.getItemCounts();
      std::shuffle(begin(itemCounts), end(itemCounts), generator);
      const auto itemIt = std::find_if(begin(itemCounts), end(itemCounts), [&hero = state.hero](const auto& itemCount) {
        return hero.canUse(itemCount.first);
      });
      if (itemIt != end(itemCounts))
        return Use{itemIt->first};
      break;
    }
    case 5:
    {
      const auto& entries = state.hero.getItemsAndSpells();
      if (!entries.empty())
      {
        if (const auto& entry = randomElement(entries); entry.conversionPoints >= 0)
          return Convert{entry.itemOrSpell};
        // few items cannot be converted, do not retry here
      }
      break;
    }
    case 6:
      if (!state.resources.spells.empty() && state.hero.hasRoomFor(Spell::Burndayraz))
        return Find{randomElement(state.resources.spells)};
      break;
    case 7:
      if (!state.resources.freeSpells.empty() && state.hero.hasRoomFor(Spell::Burndayraz))
        return FindFree{randomElement(state.resources.freeSpells)};
      break;
    case 8:
    {
      if (state.resources.altars.empty() || (state.hero.getFollowedDeity() && state.hero.getPiety() < 50))
        break;
      if (const auto altar = otherAltar())
        return Follow{*altar};
      break;
    }
    case 9:
      if (state.hero.getFollowedDeity())
      {
        auto boons = offeredBoons(*state.hero.getFollowedDeity());
        std::shuffle(begin(boons), end(boons), generator);
        auto boonIt = std::find_if(begin(boons), end(boons), [&state, &faith = state.hero.getFaith()](Boon boon) {
          return static_cast<int>(faith.getPiety()) >= faith.getCosts(boon, state.hero) &&
                 faith.isAvailable(boon, state.hero, state.visibleMonsters, state.resources);
        });
        if (boonIt != end(boons))
          return Request{*boonIt};
      }
      break;
    case 10:
      if (state.resources.pactmakerAvailable() && !state.hero.getFaith().getPact())
      {
        const auto last = state.hero.getFaith().enteredConsensus() ? Pact::LastNoConsensus : Pact::LastWithConsensus;
        return Request{static_cast<Pact>(std::uniform_int_distribution<>(0, static_cast<int>(last) - 1)(generator))};
      }
      break;
    case 11:
      if (state.hero.getFollowedDeity() && !state.resources.altars.empty() && !state.hero.has(HeroTrait::Scapegoat))
      {
        if (const auto altar = otherAltar())
          return Desecrate{*altar};
      }
      break;
    case 12:
    {
      const auto numMonsters = state.visibleMonsters.size();
      if (numMonsters > 1)
      {
        auto newTargetIndex = std::uniform_int_distribution<std::size_t>(0u, numMonsters - 2)(generator);
        if (newTargetIndex == state.activeMonster)
          newTargetIndex = (newTargetIndex + 1) % numMonsters;
        return ChangeTarget{newTargetIndex};
      }
    }
    }
    return NoOp{};
  }

  Step generateRandomValidStep(const GameState& state, bool allowTargetChange)
  {
    // Rely on at least one monster being present
    assert(state.activeMonster < state.visibleMonsters.size());
    std::uniform_int_distribution<> randomAction(0, allowTargetChange ? 12 : 11);

    while (true)
    {
      auto step = generateValidStep(state, randomAction(generator));
      if (!std::holds_alternative<NoOp>(step))
        return step;
    }
  }

  std::vector<Step> generateAllValidSteps(const GameState& state, bool allowTargetChange)
  {
    std::vector<Step> steps;
    steps.emplace_back(Attack{});
    if (state.resources.numHiddenTiles > 0)
      steps.emplace_back(Uncover{1});
    const auto& hero = state.hero;
    const auto& monsters = state.visibleMonsters;
    const bool hasMonster = state.activeMonster < monsters.size();
    for (auto& entry : hero.getItemsAndSpells())
    {
      if (const auto spell = std::get_if<Spell>(&entry.itemOrSpell))
      {
        if ((!Magic::needsMonster(*spell) && Magic::isPossible(hero, *spell, state.resources)) ||
            (hasMonster && Magic::isPossible(hero, monsters[state.activeMonster], *spell, state.resources)))
          steps.emplace_back(Cast{*spell});
      }
      else if (const auto item = std::get<Item>(entry.itemOrSpell); hero.canUse(item))
        steps.emplace_back(Use{item});
      if (entry.conversionPoints >= 0)
        steps.emplace_back(Convert{entry.itemOrSpell});
    }
    for (const auto& item : state.resources.shops)
    {
      if (hero.hasRoomFor(item) && hero.canAfford(item))
        steps.emplace_back(Buy{item});
    }
    if (hero.hasRoomFor(Spell::Burndayraz))
    {
      for (const auto& spell : state.resources.spells)
        steps.emplace_back(Find{spell});
      for (const auto& spell : state.resources.freeSpells)
        steps.emplace_back(FindFree{spell});
    }
    const auto& faith = hero.getFaith();
    if (state.resources.pactmakerAvailable() && !faith.getPact())
    {
      for (int i = 0; i < static_cast<int>(Pact::LastNoConsensus); ++i)
        steps.emplace_back(Request{static_cast<Pact>(i)});
      if (!faith.enteredConsensus())
        steps.emplace_back(Request{Pact::Consensus});
    }
    if (state.hero.getFollowedDeity())
    {
      for (const auto boon : offeredBoons(*state.hero.getFollowedDeity()))
      {
        const auto costs = hero.getBoonCosts(boon);
        if ((costs <= 0 || hero.getPiety() >= static_cast<unsigned>(costs)) &&
            faith.isAvailable(boon, hero, monsters, state.resources))
          steps.emplace_back(Request{boon});
      }
      const bool canConvert = faith.getPiety() >= 50;
      const bool canDesecrate = !hero.has(HeroTrait::Scapegoat);
      if (canConvert || canDesecrate)
      {
        for (const auto& altar : state.resources.altars)
        {
          if (const auto god = std::get_if<God>(&altar); god && *god != state.hero.getFollowedDeity())
          {
            if (canConvert)
              steps.emplace_back(Follow{*god});
            if (canDesecrate)
              steps.emplace_back(Desecrate{*god});
          }
        }
      }
    }
    else
    {
      for (const auto& altar : state.resources.altars)
      {
        if (const auto god = std::get_if<God>(&altar))
          steps.emplace_back(Follow{*god});
      }
    }
    if (allowTargetChange)
    {
      for (auto monsterIndex = 0u; monsterIndex < monsters.size(); ++monsterIndex)
      {
        if (monsterIndex != state.activeMonster)
          steps.emplace_back(ChangeTarget{monsterIndex});
      }
    }

    return steps;
  }

  bool isValid(Step step, const GameState& state)
  {
    const auto& hero = state.hero;
    const auto& monsters = state.visibleMonsters;
    return std::visit(
        overloaded{[&](Attack) { return state.activeMonster < monsters.size(); },
                   [&, &resources = state.resources](Cast cast) {
                     return hero.has(cast.spell) &&
                            (Magic::isPossible(hero, cast.spell, resources) ||
                             (state.activeMonster < monsters.size() &&
                              Magic::isPossible(hero, monsters[state.activeMonster], cast.spell, resources)));
                   },
                   [&](Uncover uncover) { return state.resources.numHiddenTiles >= uncover.numTiles; },
                   [&, &shops = state.resources.shops](Buy buy) {
                     return hero.hasRoomFor(buy.item) && static_cast<int>(hero.gold()) >= price(buy.item) &&
                            std::find(begin(shops), end(shops), buy.item) != end(shops);
                   },
                   [&](Use use) { return hero.has(use.item) && hero.canUse(use.item); },
                   [&](Convert convert) { return hero.canConvert(convert.itemOrSpell); },
                   [&hero, &spells = state.resources.spells](Find find) {
                     return hero.hasRoomFor(find.spell) &&
                            std::find(begin(spells), end(spells), find.spell) != end(spells);
                   },
                   [&hero, &spells = state.resources.freeSpells](FindFree find) {
                     return hero.hasRoomFor(find.spell) &&
                            std::find(begin(spells), end(spells), find.spell) != end(spells);
                   },
                   [piety = hero.getPiety(), current = hero.getFollowedDeity(),
                    &altars = state.resources.altars](Follow follow) {
                     return (!current || (current != follow.deity && piety >= 50)) &&
                            std::find(begin(altars), end(altars), GodOrPactmaker{follow.deity}) != end(altars);
                   },
                   [&faith = hero.getFaith(), &hero, &monsters, &resources = state.resources](Request request) {
                     if (const auto boon = std::get_if<Boon>(&request.boonOrPact))
                       return faith.isAvailable(*boon, hero, monsters, resources) &&
                              static_cast<int>(hero.getPiety()) >= faith.getCosts(*boon, hero);
                     return resources.pactmakerAvailable() && !faith.getPact() &&
                            (!faith.enteredConsensus() || std::get<Pact>(request.boonOrPact) != Pact::Consensus);
                   },
                   [current = hero.getFollowedDeity(), &altars = state.resources.altars](Desecrate desecrate) {
                     return current != desecrate.altar &&
                            std::find(begin(altars), end(altars), GodOrPactmaker{desecrate.altar}) != end(altars);
                   },
                   [&](ChangeTarget changeTarget) { return changeTarget.targetIndex < monsters.size(); },
                   [](NoOp) { return true; }},
        step);
  }

  GameState apply(const Step& step, GameState state)
  {
    auto& monsters = state.visibleMonsters;
    if (state.activeMonster >= monsters.size())
      return state;
    auto& hero = state.hero;
    auto& monster = monsters[state.activeMonster];
    std::visit(overloaded{[&](Attack) { Combat::attack(hero, monster, monsters, state.resources); },
                          [&](Cast cast) { Magic::cast(hero, monster, cast.spell, monsters, state.resources); },
                          [&](Uncover uncover) {
                            hero.recover(uncover.numTiles, monsters);
                            monster.recover(uncover.numTiles);
                            state.resources.numHiddenTiles -= uncover.numTiles;
                          },
                          [&hero, &shops = state.resources.shops](Buy buy) {
                            shops.erase(std::find(begin(shops), end(shops), buy.item));
                            hero.buy(buy.item);
                          },
                          [&](Use use) { hero.use(use.item, monsters); },
                          [&](Convert convert) { hero.convert(convert.itemOrSpell, monsters); },
                          [&hero, &spells = state.resources.spells](Find find) {
                            if (hero.receive(find.spell))
                              spells.erase(std::find(begin(spells), end(spells), find.spell));
                          },
                          [&hero, &spells = state.resources.freeSpells](FindFree find) {
                            if (hero.receiveFreeSpell(find.spell))
                              spells.erase(std::find(begin(spells), end(spells), find.spell));
                          },
                          [&](Follow follow) {
                            hero.followDeity(follow.deity, state.resources.numRevealedTiles, state.resources);
                          },
                          [&](Request request) { hero.request(request.boonOrPact, monsters, state.resources); },
                          [&hero, &monsters, &altars = state.resources.altars](Desecrate desecrate) {
                            if (hero.desecrate(desecrate.altar, monsters))
                              altars.erase(std::find(begin(altars), end(altars), GodOrPactmaker{desecrate.altar}));
                          },
                          [&state](ChangeTarget changeTarget) { state.activeMonster = changeTarget.targetIndex; },
                          [](NoOp) {}},
               step);
    monsters.erase(
        std::remove_if(begin(monsters), end(monsters), [](const auto& monster) { return monster.isDefeated(); }),
        end(monsters));
    if (state.activeMonster > monsters.size())
      state.activeMonster = 0u;
    return state;
  }

  GameState apply(const Solution& solution, GameState state)
  {
    for (const auto& step : solution)
      state = apply(step, std::move(state));
    return state;
  }

  namespace
  {
    void print_description(const std::vector<std::string>& description)
    {
      std::string s = std::accumulate(begin(description), end(description), std::string{},
                                      [](std::string a, std::string b) { return a + ", " + b; });
      if (!s.empty())
        std::cout << s.substr(2) << '\n';
    }
  } // namespace

  void print(const Solution& solution, GameState state)
  {
    print_description(describe(state.hero));
    const auto& monsters = state.visibleMonsters;
    if (state.activeMonster < monsters.size())
      print_description(describe(monsters[state.activeMonster]));
    for (const auto& step : solution)
    {
      std::cout << toString(step) << '\n';
      const auto heroBefore = state.hero;
      const auto monsterBefore =
          state.activeMonster < monsters.size() ? std::optional{monsters[state.activeMonster].getID()} : std::nullopt;
      state = apply(step, std::move(state));
      print_description(describe_diff(heroBefore, state.hero));
      if (isCombat(step))
      {
        if (monsters.empty())
          std::cout << "***** All monsters defeated *****" << std::endl;
        else if (state.activeMonster < monsters.size())
        {
          const auto& monster = monsters[state.activeMonster];
          if (monsterBefore != monster.getID())
            std::cout << "Next monster: ";
          print_description(describe(monsters[state.activeMonster]));
        }
      }
    }
  }
} // namespace solver
