#include "SolverTools.hpp"

#include "Combat.hpp"
#include "Spells.hpp"

#include <cassert>
#include <random>

template <class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

static std::mt19937 generator(std::random_device{"/dev/urandom"}());

namespace solver
{
  Step generateRandomStep()
  {
    std::uniform_int_distribution<> randomAction(0, 11);
    std::uniform_int_distribution<> randomSpell(0, static_cast<int>(Spell::Last) - 1);
    std::uniform_int_distribution<> randomShopItem(0, static_cast<int>(Item::LastShopItem) - 1);
    std::uniform_int_distribution<> randomItem(0, static_cast<int>(Item::Last) - 1);
    std::uniform_int_distribution<> randomGod(0, static_cast<int>(God::Last) - 1);
    std::uniform_int_distribution<> randomBoon(0, static_cast<int>(Boon::Last) - 1);
    std::uniform_int_distribution<> randomPact(0, static_cast<int>(Pact::LastWithConsensus) - 1);
    switch (randomAction(generator))
    {
    default:
    case 0:
      return Attack{};
    case 1:
      return Cast{static_cast<Spell>(randomSpell(generator))};
    case 2:
      return Uncover{1};
    case 3:
      return Buy{static_cast<Item>(randomShopItem(generator))};
    case 4:
      return Use{static_cast<Item>(randomItem(generator))};
    case 5:
      return Convert{static_cast<Item>(randomItem(generator))};
    case 6:
      return Convert{static_cast<Spell>(randomSpell(generator))};
    case 7:
      return Find{static_cast<Spell>(randomSpell(generator))};
    case 8:
      return Follow{static_cast<God>(randomGod(generator))};
    case 9:
      return Request{static_cast<Boon>(randomBoon(generator))};
    case 10:
      return Request{static_cast<Pact>(randomPact(generator))};
    case 11:
      return Desecrate{static_cast<God>(randomGod(generator))};
    }
  }

  Step generateRandomValidStep(const GameState& state)
  {
    // TODO: Migrate to C++20 and replace std::transform and std::shuffle with equivalent std::ranges functions

    // Rely on at least one monster being present
    assert(!state.monsters.empty());
    std::uniform_int_distribution<> randomAction(0, 10);
    std::optional<std::vector<Spell>> spells;
    std::optional<std::vector<Item>> shops;
    std::optional<std::vector<Item>> items;

    auto otherAltar = [&hero = state.hero, altars = state.resources.altars]() mutable -> std::optional<God> {
      if (altars.empty())
        return {};
      std::shuffle(begin(altars), end(altars), generator);
      if (!hero.getFollowedDeity())
        return altars.front();
      auto altarIt =
          std::find_if(begin(altars), end(altars),
                       [followedDeity = *hero.getFollowedDeity()](const auto altar) { return altar != followedDeity; });
      if (altarIt != end(altars))
        return *altarIt;
      return {};
    };

    auto randomElement = [](auto& vec) {
      assert(!vec.empty());
      return vec[std::uniform_int_distribution<>(0, vec.size() - 1)(generator)];
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
          const auto spellEntries = state.hero.getSpells();
          spells.emplace(spellEntries.size());
          std::transform(begin(spellEntries), end(spellEntries), begin(*spells),
                         [](const auto& entry) { return std::get<Spell>(entry.itemOrSpell); });
          std::shuffle(begin(*spells), end(*spells), generator);
          const auto spellIt = std::find_if(begin(*spells), end(*spells),
                                            [&hero = state.hero, &monster = state.monsters.front()](const Spell spell) {
                                              return Magic::isPossible(hero, monster, spell) ||
                                                     (!Magic::needsMonster(spell) && Magic::isPossible(hero, spell));
                                            });
          if (spellIt != end(*spells))
            return Cast{*spellIt};
        }
        break;
      case 2:
        if (state.resources.numBlackTiles > 0)
          return Uncover{1};
        break;
      case 3:
        if (!state.hero.hasRoomFor(Item::HealthPotion) || state.hero.gold() == 0)
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
          const auto itemEntries = state.hero.getItems();
          items.emplace(itemEntries.size());
          std::transform(begin(itemEntries), end(itemEntries), begin(*items),
                         [](const auto& entry) { return std::get<Item>(entry.itemOrSpell); });
          std::shuffle(begin(*items), end(*items), generator);
          const auto itemIt = std::find_if(begin(*items), end(*items),
                                           [&hero = state.hero](const Item item) { return hero.canUse(item); });
          if (itemIt != end(*items))
            return Use{*itemIt};
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
          auto boonIt =
              std::find_if(begin(boons), end(boons), [&hero = state.hero, &faith = state.hero.getFaith()](Boon boon) {
                return faith.getPiety() >= faith.getCosts(boon, hero) && faith.isAvailable(boon, hero);
              });
          if (boonIt != end(boons))
            return Request{*boonIt};
        }
        break;
      case 9:
        if (state.resources.pactMakerAvailable && !state.hero.getFaith().getPact())
        {
          const auto last = state.hero.getFaith().enteredConsensus() ? Pact::LastNoConsensus : Pact::LastWithConsensus;
          return Request{static_cast<Pact>(std::uniform_int_distribution<>(0, static_cast<int>(last) - 1)(generator))};
        }
        break;
      case 10:
        if (state.hero.getFollowedDeity() && !state.resources.altars.empty())
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
                   [&](Cast cast) {
                     return hero.has(cast.spell) &&
                            ((!state.monsters.empty() && Magic::isPossible(hero, state.monsters.front(), cast.spell)) ||
                             (!Magic::needsMonster(cast.spell) && Magic::isPossible(hero, cast.spell)));
                   },
                   [&](Uncover uncover) { return state.resources.numBlackTiles >= uncover.numTiles; },
                   [&, &shops = state.resources.shops](Buy buy) {
                     return hero.hasRoomFor(buy.item) && hero.gold() >= price(buy.item) &&
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
                            std::find(begin(altars), end(altars), follow.deity) != end(altars);
                   },
                   [&faith = hero.getFaith(), &hero, &pactMaker = state.resources.pactMakerAvailable](Request request) {
                     if (const auto boon = std::get_if<Boon>(&request.boonOrPact))
                       return faith.isAvailable(*boon, hero) && hero.getPiety() >= faith.getCosts(*boon, hero);
                     return pactMaker && !faith.getPact() &&
                            (!faith.enteredConsensus() || std::get<Pact>(request.boonOrPact) != Pact::Consensus);
                   },
                   [current = hero.getFollowedDeity(), &altars = state.resources.altars](Desecrate desecrate) {
                     return current != desecrate.altar &&
                            std::find(begin(altars), end(altars), desecrate.altar) != end(altars);
                   }},
        step);
  }

  GameState apply(const Step& step, GameState state)
  {
    if (state.monsters.empty())
      return state;
    auto& hero = state.hero;
    auto& monster = state.monsters.front();
    std::visit(overloaded{[&](Attack) { Combat::attack(hero, monster); },
                          [&](Cast cast) { Magic::cast(hero, monster, cast.spell); },
                          [&](Uncover uncover) {
                            hero.recover(uncover.numTiles);
                            monster.recover(uncover.numTiles);
                            state.resources.numBlackTiles -= uncover.numTiles;
                          },
                          [&hero, &shops = state.resources.shops](Buy buy) {
                            shops.erase(std::find(begin(shops), end(shops), buy.item));
                            hero.buy(buy.item);
                          },
                          [&](Use use) { hero.use(use.item); },
                          [&](Convert convert) { hero.convert(convert.itemOrSpell); },
                          [&hero, &spells = state.resources.spells](Find find) {
                            spells.erase(std::find(begin(spells), end(spells), find.spell));
                            hero.receive(find.spell);
                          },
                          [&](Follow follow) { hero.followDeity(follow.deity); },
                          [&](Request request) { hero.request(request.boonOrPact); },
                          [&hero, &altars = state.resources.altars](Desecrate desecrate) {
                            altars.erase(std::find(begin(altars), end(altars), desecrate.altar));
                            hero.desecrate(desecrate.altar);
                          }},
               step);
    while (!state.monsters.empty() && state.monsters.front().isDefeated())
      state.monsters.erase(state.monsters.begin());
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
                                      [](auto& a, auto& b) { return a + ", " + b; });
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
      const int poolSize = state.monsters.size();
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
