#include "solver/SolverTools.hpp"

#include "engine/Combat.hpp"
#include "engine/Magic.hpp"

#include <cassert>
#include <iostream>
#include <random>
#include <variant>

namespace solver
{
  thread_local std::mt19937 generator{std::random_device{"/dev/urandom"}()};

  Step generateStep(const GameState& state, int stepTypeIndex)
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
      return Attack{};
    case 1:
    {
      if (state.visibleMonsters.empty())
        break;
      auto spellCounts = state.hero.getSpellCounts();
      std::shuffle(begin(spellCounts), end(spellCounts), generator);
      const auto spellIt = std::find_if(begin(spellCounts), end(spellCounts),
                                        [&state, &monster = state.visibleMonsters.front()](const auto& spellCount) {
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
    }
    return NoOp{};
  }

  Step generateRandomValidStep(const GameState& state)
  {
    // Rely on at least one monster being present
    assert(!state.visibleMonsters.empty());
    std::uniform_int_distribution<> randomAction(0, 11);

    while (true)
    {
      auto step = generateStep(state, randomAction(generator));
      if (!std::holds_alternative<NoOp>(step))
        return step;
    }
  }

  bool isValid(Step step, const GameState& state)
  {
    const auto& hero = state.hero;
    return std::visit(
        overloaded{[&](Attack) { return !state.visibleMonsters.empty(); },
                   [&, &monsters = state.visibleMonsters, &resources = state.resources](Cast cast) {
                     return hero.has(cast.spell) &&
                            (Magic::isPossible(hero, cast.spell, resources) ||
                             (!monsters.empty() && Magic::isPossible(hero, monsters.front(), cast.spell, resources)));
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
                   [&faith = hero.getFaith(), &hero, &resources = state.resources,
                    &monsters = state.visibleMonsters](Request request) {
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
                   [](NoOp) { return true; }},
        step);
  }

  GameState apply(const Step& step, GameState state)
  {
    if (state.visibleMonsters.empty())
      return state;
    auto& hero = state.hero;
    auto& monster = state.visibleMonsters.front();
    std::visit(
        overloaded{
            [&](Attack) { Combat::attack(hero, monster, state.visibleMonsters, state.resources); },
            [&](Cast cast) { Magic::cast(hero, monster, cast.spell, state.visibleMonsters, state.resources); },
            [&](Uncover uncover) {
              hero.recover(uncover.numTiles, state.visibleMonsters);
              monster.recover(uncover.numTiles);
              state.resources.numHiddenTiles -= uncover.numTiles;
            },
            [&hero, &shops = state.resources.shops](Buy buy) {
              shops.erase(std::find(begin(shops), end(shops), buy.item));
              hero.buy(buy.item);
            },
            [&](Use use) { hero.use(use.item, state.visibleMonsters); },
            [&](Convert convert) { hero.convert(convert.itemOrSpell, state.visibleMonsters); },
            [&hero, &spells = state.resources.spells](Find find) {
              if (hero.receive(find.spell))
                spells.erase(std::find(begin(spells), end(spells), find.spell));
            },
            [&hero, &spells = state.resources.freeSpells](FindFree find) {
              if (hero.receiveFreeSpell(find.spell))
                spells.erase(std::find(begin(spells), end(spells), find.spell));
            },
            [&](Follow follow) { hero.followDeity(follow.deity, state.resources.numRevealedTiles, state.resources); },
            [&](Request request) { hero.request(request.boonOrPact, state.visibleMonsters, state.resources); },
            [&hero, &monsters = state.visibleMonsters, &altars = state.resources.altars](Desecrate desecrate) {
              if (hero.desecrate(desecrate.altar, monsters))
                altars.erase(std::find(begin(altars), end(altars), GodOrPactmaker{desecrate.altar}));
            },
            [](NoOp) {}},
        step);
    state.visibleMonsters.erase(std::remove_if(begin(state.visibleMonsters), end(state.visibleMonsters),
                                               [](const auto& monster) { return monster.isDefeated(); }),
                                end(state.visibleMonsters));
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

  void print(Solution solution, GameState state)
  {
    print_description(describe(state.hero));
    if (!state.visibleMonsters.empty())
      print_description(describe(state.visibleMonsters.front()));
    for (const auto& step : solution)
    {
      std::cout << toString(step) << '\n';
      const auto heroBefore = state.hero;
      const auto poolSize = state.visibleMonsters.size();
      state = apply(step, std::move(state));
      print_description(describe_diff(heroBefore, state.hero));
      if (isCombat(step))
      {
        if (state.visibleMonsters.empty())
          std::cout << "***** All enemies defeated *****" << std::endl;
        else
        {
          if (state.visibleMonsters.size() != poolSize)
            std::cout << "Next enemy: ";
          print_description(describe(state.visibleMonsters.front()));
        }
      }
    }
  }
} // namespace solver
