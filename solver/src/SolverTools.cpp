#include "solver/SolverTools.hpp"

#include "engine/Combat.hpp"
#include "engine/Magic.hpp"

#include <cassert>
#include <iostream>
#include <random>

static std::mt19937 generator(std::random_device{"/dev/urandom"}());

namespace solver
{
  Step generateRandomValidStep(const GameState& state)
  {
    // TODO: Replace std::transform and std::shuffle with equivalent std::ranges functions

    // Rely on at least one monster being present
    assert(!state.monsters.empty());
    std::uniform_int_distribution<> randomAction(0, 10);
    std::optional<std::vector<Spell>> spells;
    std::optional<std::vector<Item>> shops;
    std::optional<std::vector<Item>> items;

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

    auto randomElement = [](auto& vec) {
      assert(!vec.empty());
      return vec[std::uniform_int_distribution<unsigned>(0, vec.size() - 1)(generator)];
    };

    while (true)
    {
      switch (randomAction(generator))
      {
      default:
      case 0:
        return Attack{};
      case 1:
        if (!spells)
        {
          auto spellCounts = state.hero.getSpellCounts();
          std::shuffle(begin(spellCounts), end(spellCounts), generator);
          const auto spellIt = std::find_if(begin(spellCounts), end(spellCounts),
                                            [&state, &monster = state.monsters.front()](const auto& spellCount) {
                                              const Spell spell = spellCount.first;
                                              return Magic::isPossible(state.hero, monster, spell, state.resources);
                                            });
          if (spellIt != end(spellCounts))
            return Cast{spellIt->first};
        }
        break;
      case 2:
        if (state.resources.numHiddenTiles > 0)
          return Uncover{1};
        break;
      case 3:
        if (!state.hero.hasRoomFor(Potion::HealthPotion) || state.hero.gold() == 0)
          break;
        if (!shops)
        {
          shops = state.resources.shops;
          std::shuffle(begin(*shops), end(*shops), generator);
          const auto shopIt = std::find_if(begin(*shops), end(*shops), [&hero = state.hero](const Item item) {
            return hero.hasRoomFor(item) && hero.canAfford(item);
          });
          if (shopIt != end(*shops))
            return Buy{*shopIt};
        }
        break;
      case 4:
        if (!items)
        {
          auto itemCounts = state.hero.getItemCounts();
          std::shuffle(begin(itemCounts), end(itemCounts), generator);
          const auto itemIt =
              std::find_if(begin(itemCounts), end(itemCounts),
                           [&hero = state.hero](const auto& itemCount) { return hero.canUse(itemCount.first); });
          if (itemIt != end(itemCounts))
            return Use{itemIt->first};
        }
        break;
      case 5:
      {
        const auto& entries = state.hero.getItemsAndSpells();
        if (!entries.empty())
        {
          const auto& entry = randomElement(entries);
          if (entry.conversionPoints >= 0)
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
      {
        if (state.resources.altars.empty() || (state.hero.getFollowedDeity() && state.hero.getPiety() < 50))
          break;
        const auto altar = otherAltar();
        if (altar)
          return Follow{*altar};
        break;
      }
      case 8:
        if (state.hero.getFollowedDeity())
        {
          auto boons = offeredBoons(*state.hero.getFollowedDeity());
          std::shuffle(begin(boons), end(boons), generator);
          auto boonIt = std::find_if(begin(boons), end(boons), [&state, &faith = state.hero.getFaith()](Boon boon) {
            return static_cast<int>(faith.getPiety()) >= faith.getCosts(boon, state.hero) &&
                   faith.isAvailable(boon, state.hero, state.monsters, state.resources);
          });
          if (boonIt != end(boons))
            return Request{*boonIt};
        }
        break;
      case 9:
        if (state.resources.pactmakerAvailable() && !state.hero.getFaith().getPact())
        {
          const auto last = state.hero.getFaith().enteredConsensus() ? Pact::LastNoConsensus : Pact::LastWithConsensus;
          return Request{static_cast<Pact>(std::uniform_int_distribution<>(0, static_cast<int>(last) - 1)(generator))};
        }
        break;
      case 10:
        if (state.hero.getFollowedDeity() && !state.resources.altars.empty() && !state.hero.has(HeroTrait::Scapegoat))
        {
          const auto altar = otherAltar();
          if (altar)
            return Desecrate{*altar};
        }
        break;
      }
    }
  }

  bool isValid(Step step, const GameState& state)
  {
    const auto& hero = state.hero;
    return std::visit(
        overloaded{[&](Attack) { return !state.monsters.empty(); },
                   [&, &monsters = state.monsters, &resources = state.resources](Cast cast) {
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
                   [piety = hero.getPiety(), current = hero.getFollowedDeity(),
                    &altars = state.resources.altars](Follow follow) {
                     return (!current || (current != follow.deity && piety >= 50)) &&
                            std::find(begin(altars), end(altars), GodOrPactmaker{follow.deity}) != end(altars);
                   },
                   [&faith = hero.getFaith(), &hero, &resources = state.resources,
                    &monsters = state.monsters](Request request) {
                     if (const auto boon = std::get_if<Boon>(&request.boonOrPact))
                       return faith.isAvailable(*boon, hero, monsters, resources) &&
                              static_cast<int>(hero.getPiety()) >= faith.getCosts(*boon, hero);
                     return resources.pactmakerAvailable() && !faith.getPact() &&
                            (!faith.enteredConsensus() || std::get<Pact>(request.boonOrPact) != Pact::Consensus);
                   },
                   [current = hero.getFollowedDeity(), &altars = state.resources.altars](Desecrate desecrate) {
                     return current != desecrate.altar &&
                            std::find(begin(altars), end(altars), GodOrPactmaker{desecrate.altar}) != end(altars);
                   }},
        step);
  }

  GameState apply(const Step& step, GameState state)
  {
    if (state.monsters.empty())
      return state;
    auto& hero = state.hero;
    auto& monster = state.monsters.front();
    std::visit(overloaded{[&](Attack) { Combat::attack(hero, monster, state.monsters, state.resources); },
                          [&](Cast cast) { Magic::cast(hero, monster, cast.spell, state.monsters, state.resources); },
                          [&](Uncover uncover) {
                            hero.recover(uncover.numTiles, state.monsters);
                            monster.recover(uncover.numTiles);
                            state.resources.numHiddenTiles -= uncover.numTiles;
                          },
                          [&hero, &shops = state.resources.shops](Buy buy) {
                            shops.erase(std::find(begin(shops), end(shops), buy.item));
                            hero.buy(buy.item);
                          },
                          [&](Use use) { hero.use(use.item, state.monsters); },
                          [&](Convert convert) { hero.convert(convert.itemOrSpell, state.monsters); },
                          [&hero, &spells = state.resources.spells](Find find) {
                            spells.erase(std::find(begin(spells), end(spells), find.spell));
                            hero.receive(find.spell);
                          },
                          [&](Follow follow) { hero.followDeity(follow.deity, state.resources.numRevealedTiles); },
                          [&](Request request) { hero.request(request.boonOrPact, state.monsters, state.resources); },
                          [&hero, &monsters = state.monsters, &altars = state.resources.altars](Desecrate desecrate) {
                            if (hero.desecrate(desecrate.altar, monsters))
                              altars.erase(std::find(begin(altars), end(altars), GodOrPactmaker{desecrate.altar}));
                          }},
               step);
    state.monsters.erase(std::remove_if(begin(state.monsters), end(state.monsters),
                                        [](const auto& monster) { return monster.isDefeated(); }),
                         end(state.monsters));
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
    if (!state.monsters.empty())
      print_description(describe(state.monsters.front()));
    for (const auto& step : solution)
    {
      std::cout << toString(step) << '\n';
      auto heroBefore = state.hero;
      const unsigned poolSize = state.monsters.size();
      state = apply(step, std::move(state));
      print_description(describe_diff(heroBefore, state.hero));
      if (isCombat(step))
      {
        if (state.monsters.empty())
          std::cout << "***** All enemies defeated *****" << std::endl;
        else
        {
          if (state.monsters.size() != poolSize)
            std::cout << "Next enemy: ";
          print_description(describe(state.monsters.front()));
        }
      }
    }
  }
} // namespace solver
