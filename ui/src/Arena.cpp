#include "ui/Arena.hpp"

#include "ui/History.hpp"

#include "engine/Combat.hpp"
#include "engine/Hero.hpp"
#include "engine/Items.hpp"
#include "engine/Magic.hpp"
#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"

#include "imgui.h"

namespace ui
{
  using namespace std::string_literals;

  void Arena::runAttack(const State& state)
  {
    const Monster* activeMonster = state.monster();
    if (activeMonster && !activeMonster->isDefeated())
    {
      addActionButton(
          state, "Attack", false, "Attack "s + state.monster()->getName(),
          [](State& state) { return Combat::attack(state.hero, *state.monster(), state.monsterPool, state.resources); },
          result);
    }
    else
      disabledButton("Attack", "No target");
  }

  void Arena::runCastPopup(const State& state)
  {
    const auto spellCounts = state.hero.getSpellCounts();
    if (spellCounts.empty())
    {
      disabledButton("Cast", "No spells");
      return;
    }
    ImGui::Button("Cast");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("CastPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("CastPopup"))
    {
      const bool withMonster = state.monster() && !state.monster()->isDefeated();
      ImGui::TextUnformatted("Spells");
      ImGui::Separator();
      int index = -1;
      for (const auto& [spell, _] : spellCounts)
      {
        const bool isSelected = ++index == selectedPopupItem;
        const bool possible = Magic::isPossible(state.hero, spell, state.resources) ||
                              (withMonster && Magic::isPossible(state.hero, *state.monster(), spell, state.resources));
        const auto costs = Magic::spellCosts(spell, state.hero);
        const std::string label = toString(spell) + " ("s + std::to_string(costs) + " MP)";
        if (!possible)
        {
          ImGui::TextColored(colorUnavailable, "%s", label.c_str());
          continue;
        }
        const std::string historyTitle = "Cast "s + toString(spell);
        auto cast = [spell = spell, withMonster](State& state) {
          if (withMonster)
            return Magic::cast(state.hero, *state.monster(), spell, state.monsterPool, state.resources);
          Magic::cast(state.hero, spell, state.monsterPool, state.resources);
          return Summary::None;
        };
        if (addPopupAction(state, label, historyTitle, cast, isSelected, result))
          selectedPopupItem = index;
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Arena::runUseItemPopup(const State& state)
  {
    const auto itemCounts = state.hero.getItemCounts();
    if (itemCounts.empty())
    {
      disabledButton("Use", "No items");
      return;
    }
    ImGui::Button("Use");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("UseItemPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("UseItemPopup"))
    {
      int index = -1;
      for (const auto& [item, count] : itemCounts)
      {
        const bool isSelected = ++index == selectedPopupItem;
        const std::string historyTitle = "Use "s + toString(item);
        std::string label = toString(item);
        if (count > 1)
          label += " (x" + std::to_string(count) + ")";
        if (state.hero.canUse(item))
        {
          if (addPopupAction(
                  state, std::move(label), historyTitle,
                  [item = item](State& state) {
                    state.hero.use(item, state.monsterPool);
                    return Summary::None;
                  },
                  isSelected, result))
            selectedPopupItem = index;
        }
        else if (auto monster = state.monster(); monster && !monster->isDefeated() && state.hero.canUse(item, *monster))
        {
          if (addPopupAction(
                  state, std::move(label), historyTitle,
                  [item = item](State& state) {
                    state.hero.use(item, *state.monster(), state.monsterPool);
                    return Summary::None;
                  },
                  isSelected, result))
            selectedPopupItem = index;
        }
        else
          ImGui::TextColored(colorUnavailable, "%s", label.c_str());
      }

      const auto& inventory = state.hero.getItemsAndSpells();
      const bool canCompress =
          state.hero.has(AlchemistSeal::CompressionSeal) &&
          std::find_if(begin(inventory), end(inventory), [](auto& entry) { return !entry.isSmall; }) != end(inventory);
      if (canCompress)
      {
        ImGui::Separator();
        ImGui::TextUnformatted("Compress");
        ImGui::Separator();
        std::set<ItemOrSpell> alreadySeen;
        for (const auto& entry : inventory)
        {
          if (entry.isSmall)
            continue;
          if (!alreadySeen.insert(entry.itemOrSpell).second)
            continue;
          const bool isSelected = ++index == selectedPopupItem;
          const std::string label = toString(entry.itemOrSpell);
          const std::string historyTitle = "Compress " + label;
          if (addPopupAction(
                  state, label, historyTitle,
                  [itemOrSpell = entry.itemOrSpell](State& state) {
                    state.hero.useCompressionSealOn(itemOrSpell);
                    return Summary::None;
                  },
                  isSelected, result))
            selectedPopupItem = index;
        }
      }

      const bool canTransmute = state.hero.has(AlchemistSeal::TransmutationSeal) &&
                                std::find_if(begin(inventory), end(inventory), [hero = state.hero](auto& entry) {
                                  const auto item = std::get_if<Item>(&entry.itemOrSpell);
                                  return !item || hero.sellingPrice(*item) >= 0;
                                }) != end(inventory);
      if (canTransmute)
      {
        ImGui::Separator();
        ImGui::TextUnformatted("Transmute");
        ImGui::Separator();
        for (const auto& [item, count] : state.hero.getItemCounts())
        {
          // If there's only one transmuation seal, cannot transmute the seal itself
          if (item == Item{AlchemistSeal::TransmutationSeal} && count == 1)
            continue;
          const auto price = state.hero.sellingPrice(item);
          if (price < 0)
            continue;
          const bool isSelected = ++index == selectedPopupItem;
          const std::string label = toString(item) + " ("s + std::to_string(price) + " gold)";
          const std::string historyTitle = "Transmute " + label;
          if (addPopupAction(
                  state, label, historyTitle,
                  [item = item](State& state) {
                    state.hero.useTransmutationSealOn(item, state.monsterPool);
                    return Summary::None;
                  },
                  isSelected, result))
            selectedPopupItem = index;
        }
      }

      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Arena::runConvertItemPopup(const State& state)
  {
    const auto itemsGrouped = state.hero.getItemsGrouped();
    const auto spells = state.hero.getSpells();
    if (itemsGrouped.empty() && spells.empty())
    {
      disabledButton("Convert", "Empty inventory");
      return;
    }
    ImGui::Button("Convert");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("ConvertPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("ConvertPopup"))
    {
      int index = -1;
      if (!itemsGrouped.empty())
      {
        ImGui::TextUnformatted("Items");
        ImGui::Separator();
        for (const auto& [itemEntry, count] : itemsGrouped)
        {
          const bool isSelected = ++index == selectedPopupItem;
          const auto item = std::get<Item>(itemEntry.itemOrSpell);
          if (itemEntry.conversionPoints >= 0)
          {
            std::string label = toString(item) + " ("s + std::to_string(itemEntry.conversionPoints) + " CP)";
            const std::string historyTitle = "Convert " + label;
            if (count > 1)
              label += " (x"s + std::to_string(count) + ')';
            if (addPopupAction(
                    state, label, historyTitle,
                    [item](State& state) {
                      state.hero.convert(item, state.monsterPool);
                      return Summary::None;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
          else
            ImGui::TextColored(colorUnavailable, "%s", toString(item));
        }
        if (!spells.empty())
          ImGui::Separator();
      }
      if (!spells.empty())
      {
        ImGui::TextUnformatted("Spells");
        ImGui::Separator();
        for (const auto& entry : spells)
        {
          const bool isSelected = ++index == selectedPopupItem;
          const auto spell = std::get<Spell>(entry.itemOrSpell);
          if (entry.conversionPoints >= 0)
          {
            const std::string label = toString(spell) + " ("s + std::to_string(entry.conversionPoints) + " CP)";
            const std::string historyTitle = "Convert " + label;
            if (addPopupAction(
                    state, label, historyTitle,
                    [spell](State& state) {
                      state.hero.convert(spell, state.monsterPool);
                      return Summary::None;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
          else
            ImGui::TextColored(colorUnavailable, "%s", toString(spell));
        }
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Arena::runShopPopup(const State& state)
  {
    const auto shops = state.resources().shops;
    const auto numPotionShops = state.resources().numPotionShops;
    if (shops.empty() && numPotionShops == 0)
    {
      disabledButton("Buy", "No shops");
      return;
    }
    ImGui::Button("Buy");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("ShopPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("ShopPopup"))
    {
      int index = 0;
      if (!shops.empty())
      {
        if (state.hero.has(AlchemistSeal::TranslocationSeal))
          ImGui::Checkbox("Use Translocation Seal", &useTranslocationSeal);
        else
          useTranslocationSeal = false;
        const bool allItemsLarge = state.hero.has(HeroTrait::RegalSize);
        const auto numFreeSlots =
            state.hero.numFreeSmallInventorySlots() + (useTranslocationSeal ? (allItemsLarge ? 5 : 1) : 0);
        if (numFreeSlots < 5)
        {
          if (numFreeSlots == 0 || allItemsLarge)
            ImGui::TextColored(colorUnavailable, "No room in inventory");
          else
            ImGui::TextColored(colorUnavailable, "No room for large items");
          ImGui::Separator();
        }
        for (size_t shopIndex = 0u; shopIndex < shops.size(); ++shopIndex)
        {
          const auto item = shops[shopIndex];
          const bool isSelected = ++index == selectedPopupItem;
          const int price = state.hero.buyingPrice(item);
          const std::string label = toString(item) + " ("s + std::to_string(price) + " gold)";
          if (useTranslocationSeal && (isSmall(item) || numFreeSlots >= 5))
          {
            const std::string historyTitle = "Translocate " + label;
            if (addPopupAction(
                    state, label, historyTitle,
                    [item, shopIndex = static_cast<long>(shopIndex)](State& state) {
                      if (state.hero.useTranslocationSealOn(item))
                      {
                        auto& shops = state.resources().shops;
                        shops.erase(begin(shops) + shopIndex);
                        return Summary::None;
                      }
                      return Summary::NotPossible;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
          else if (state.hero.canBuy(item))
          {
            const std::string historyTitle = "Buy " + label;
            if (addPopupAction(
                    state, label, historyTitle,
                    [item, shopIndex = static_cast<long>(shopIndex)](State& state) {
                      if (state.hero.buy(item))
                      {
                        auto& shops = state.resources().shops;
                        shops.erase(begin(shops) + shopIndex);
                      }
                      return Summary::None;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
          else
            ImGui::TextColored(colorUnavailable, "%s", label.c_str());
        }
      }
      const std::string title = "Potion Shop (x"s + std::to_string(numPotionShops) + ")";
      if (numPotionShops > 0 && ImGui::BeginMenu(title.c_str()))
      {
        for (int potionIndex = static_cast<int>(0); potionIndex <= static_cast<int>(Potion::LastInShop); ++potionIndex)
        {
          const bool isSelected = ++index == selectedPopupItem;
          const auto potion = static_cast<Potion>(potionIndex);
          const int price = state.hero.buyingPrice(potion);
          const std::string label = toString(potion) + " ("s + std::to_string(price) + " gold)";
          const std::string historyTitle = "Buy " + label;
          if (state.hero.canBuy(potion))
          {
            if (addPopupAction(
                    state, label, historyTitle,
                    [potion](State& state) {
                      if (state.hero.buy(potion))
                        --state.resources().numPotionShops;
                      return Summary::None;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
          else
            ImGui::TextColored(colorUnavailable, "%s", label.c_str());
        }
        ImGui::EndMenu();
      }
      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Arena::runFaithPopup(const State& state)
  {
    const auto [altars, canMakePact] = [&state] {
      std::vector<God> altars;
      bool canMakePact = false;
      for (auto godOrPactmaker : state.resources().altars)
      {
        if (auto god = std::get_if<God>(&godOrPactmaker))
          altars.push_back(*god);
        else
          canMakePact = !state.hero.getFaith().getPact();
      }
      return std::tuple{std::move(altars), canMakePact};
    }();
    const bool scapegoat = state.hero.has(HeroTrait::Scapegoat);
    if (!canMakePact)
    {
      if (scapegoat && std::find(begin(altars), end(altars), *state.hero.getFollowedDeity()) == end(altars))
      {
        disabledButton("Faith", "Altar of followed deity not available");
        return;
      }
      if (altars.empty())
      {
        disabledButton("Faith", "No altars");
        return;
      }
    }
    ImGui::Button("Faith");
    if (ImGui::IsItemActive())
    {
      ImGui::OpenPopup("FaithPopup");
      selectedPopupItem = -1;
    }
    if (ImGui::BeginPopup("FaithPopup"))
    {
      int index = 0;
      const auto following = state.hero.getFollowedDeity();
      if (following && std::find(begin(altars), end(altars), *following) != end(altars))
      {
        ImGui::TextUnformatted("Request Boon");
        ImGui::Separator();
        for (const Boon boon : offeredBoons(*following))
        {
          const bool isSelected = ++index == selectedPopupItem;
          const int costs = state.hero.getBoonCosts(boon);
          const std::string label = toString(boon) + " ("s + std::to_string(costs) + ")";
          if (state.hero.getFaith().isAvailable(boon, state.hero, state.monsterPool, state.resources))
          {
            const std::string historyTitle = "Request "s + toString(boon);
            if (addPopupAction(
                    state, label, historyTitle,
                    [boon](State& state) {
                      state.hero.getFaith().request(boon, state.hero, state.monsterPool, state.resources);
                      return Summary::None;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
          else
            ImGui::TextColored(colorUnavailable, "%s", label.c_str());
        }
      }
      const bool haveAvailableAltars =
          !scapegoat && !altars.empty() &&
          (!following || altars.size() > static_cast<unsigned>(std::count(begin(altars), end(altars), *following)));
      if (haveAvailableAltars)
      {
        if (following)
        {
          ImGui::Separator();
          ImGui::TextUnformatted("Convert");
        }
        else
          ImGui::TextUnformatted("Worship");
        ImGui::Separator();
        for (const God deity : altars)
        {
          if (following == deity)
            continue;
          const bool isSelected = ++index == selectedPopupItem;
          const std::string label = "Follow "s + toString(deity);
          if (state.hero.getFaith().canFollow(deity, state.hero))
          {
            if (addPopupAction(
                    state, toString(deity), label,
                    [deity](State& state) {
                      return state.hero.getFaith().followDeity(deity, state.hero, state.resources.numRevealedTiles,
                                                               state.resources)
                                 ? Summary::None
                                 : Summary::NotPossible;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
          else
            ImGui::TextColored(colorUnavailable, "%s", label.c_str());
        }

        if (following)
        {
          ImGui::Separator();
          ImGui::TextUnformatted("Desecrate");
          ImGui::Separator();
          for (size_t altarIndex = 0; altarIndex < altars.size(); ++altarIndex)
          {
            const God god = altars[altarIndex];
            if (following == god)
              continue;
            const bool isSelected = ++index == selectedPopupItem;
            if (addPopupAction(
                    state, toString(god), "Desecrate "s + toString(god) + "'s altar",
                    [god, altarIndex = static_cast<long>(altarIndex)](State& state) {
                      if (state.hero.desecrate(god, state.monsterPool))
                      {
                        auto& altars = state.resources().altars;
                        altars.erase(begin(altars) + altarIndex);
                      }
                      return Summary::Safe;
                    },
                    isSelected, result))
              selectedPopupItem = index;
          }
        }
      }
      if (canMakePact)
      {
        if (haveAvailableAltars)
          ImGui::Separator();
        ImGui::TextUnformatted("Pactmaker");
        ImGui::Separator();
        const int last = static_cast<int>(state.hero.getFaith().enteredConsensus() ? Pact::LastNoConsensus
                                                                                   : Pact::LastWithConsensus);
        for (int pactIndex = 0; pactIndex <= last; ++pactIndex)
        {
          const Pact pact = static_cast<Pact>(pactIndex);
          const bool isSelected = ++index == selectedPopupItem;
          const std::string label = toString(pact);
          const std::string historyTitle = "Enter " + label;
          if (addPopupAction(
                  state, label, historyTitle,
                  [pact](State& state) {
                    state.hero.request(pact, state.monsterPool, state.resources);
                    return Summary::Safe;
                  },
                  isSelected, result))
            selectedPopupItem = index;
        }
      }

      if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
        ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
    }
  }

  void Arena::runUncoverTiles(const State& state)
  {
    const auto numHidden = state.resources.numHiddenTiles;
    if (numHidden > 0)
    {
      addActionButton(
          state, "Reveal Tile",
          [](State& state) {
            state.resources.revealTile();
            state.heroAndMonsterRecovery(1);
            return state.hero.isDefeated() ? Summary::Death : Summary::Safe;
          },
          result);

      const auto numSquares = std::min(numHidden, state.hero.numSquaresForFullRecovery());
      if (numSquares > 1)
      {
        ImGui::SameLine();
        const std::string label = "Reveal " + std::to_string(numSquares) + " Tiles";
        addActionButton(
            state, label,
            [numSquares](State& state) {
              state.resources.revealTiles(numSquares);
              state.heroAndMonsterRecovery(numSquares);
              return Summary::Safe;
            },
            result);
      }
    }
  }

  void Arena::runPickupResource(const State& state)
  {
    const auto& visible = state.resources.visible;
    const bool haveSpells = !visible.spells.empty() || !visible.freeSpells.empty();
    if (visible.numHealthPotions == 0 && visible.numManaPotions == 0 && visible.numAttackBoosters == 0 &&
        visible.numManaBoosters == 0 && visible.numHealthBoosters == 0 && visible.numGoldPiles == 0 && !haveSpells &&
        visible.onGround.empty())
    {
      disabledButton("Pick up", "Nothing here!");
    }
    else
    {
      ImGui::Button("Pick up");
      if (ImGui::IsItemActive())
      {
        ImGui::OpenPopup("PickupPopup");
        selectedPopupItem = -1;
      }
      if (ImGui::BeginPopup("PickupPopup"))
      {
        int index = 0;
        auto addPotionPickupAction = [&, this](auto number, ItemOrSpell item) {
          if (visible.*number > 0)
          {
            const std::string label = toString(item) + " (x"s + std::to_string(visible.*number) + ")";
            const std::string historyTitle = "Pick up "s + toString(item);
            auto action = [number, item](State& state) {
              if (state.hero.receive(item))
                --(state.resources.visible.*number);
              return Summary::None;
            };
            if (addPopupAction(state, std::move(label), std::move(historyTitle), std::move(action),
                               ++index == selectedPopupItem, result))
              selectedPopupItem = index;
          }
        };
        auto addGenericPickupAction = [&, this](auto name, auto number, auto heroAction) {
          if (visible.*number > 0)
          {
            std::string label = name + " (x"s + std::to_string(visible.*number) + ")";
            std::string historyTitle = "Pick up "s + name;
            auto action = [number, heroAction](State& state) {
              (state.hero.*heroAction)();
              --(state.resources.visible.*number);
              return Summary::Safe;
            };
            if (addPopupAction(state, std::move(label), std::move(historyTitle), std::move(action),
                               ++index == selectedPopupItem, result))
              selectedPopupItem = index;
          }
        };
        auto addPickupSpellAction = [&, this](Spell spell, bool isFree) {
          const bool isSelected = ++index == selectedPopupItem;
          std::string label = toString(spell);
          if (isFree)
            label += " (free)";
          if (addPopupAction(
                  state, label, "Pick up " + label,
                  [spell, isFree](State& state) {
                    const bool success = isFree ? state.hero.receiveFreeSpell(spell) : state.hero.receive(spell);
                    if (success)
                    {
                      auto& spells = isFree ? state.resources().freeSpells : state.resources().spells;
                      auto spellIt = std::find(begin(spells), end(spells), spell);
                      if (spellIt != end(spells))
                        spells.erase(spellIt);
                    }
                    return Summary::None;
                  },
                  isSelected, result))
            selectedPopupItem = index;
        };
        auto addPickupItemAction = [&, this](Item item) {
          const bool isSelected = ++index == selectedPopupItem;
          if (addPopupAction(
                  state, toString(item), "Pick up "s + toString(item),
                  [item](State& state) {
                    if (state.hero.receive(item))
                    {
                      auto& items = state.resources().onGround;
                      auto itemIt = std::find(begin(items), end(items), item);
                      if (itemIt != end(items))
                        items.erase(itemIt);
                    }
                    return Summary::None;
                  },
                  isSelected, result))
            selectedPopupItem = index;
        };
        addGenericPickupAction("Attack Booster", &ResourceSet::numAttackBoosters, &Hero::addAttackBonus);
        addGenericPickupAction("Health Booster", &ResourceSet::numHealthBoosters, &Hero::addHealthBonus);
        addGenericPickupAction("Mana Booster", &ResourceSet::numManaBoosters, &Hero::addManaBonus);
        addGenericPickupAction("Gold Pile", &ResourceSet::numGoldPiles, &Hero::collectGoldPile);

        bool cannotTakePotion = false;
        if (visible.numHealthPotions > 0)
        {
          if (state.hero.hasRoomFor(Potion::HealthPotion))
            addPotionPickupAction(&ResourceSet::numHealthPotions, Potion::HealthPotion);
          else
            cannotTakePotion = true;
        }
        if (visible.numManaPotions > 0)
        {
          if (state.hero.hasRoomFor(Potion::ManaPotion))
            addPotionPickupAction(&ResourceSet::numManaPotions, Potion::ManaPotion);
          else
            cannotTakePotion = true;
        }

        if (cannotTakePotion)
          ImGui::TextColored(colorUnavailable, "No room in inventory");

        const bool cannotTakeSpell = haveSpells && !state.hero.hasRoomFor(Spell::Burndayraz);
        if (cannotTakeSpell)
        {
          if (!cannotTakePotion)
            ImGui::TextColored(colorUnavailable, "No room for spells");
        }
        else if (haveSpells && ImGui::BeginMenu("Spells"))
        {
          // Iterate over copies, as entries might be erased
          auto spells = visible.spells;
          for (Spell spell : spells)
            addPickupSpellAction(spell, false);
          spells = visible.freeSpells;
          for (Spell spell : spells)
            addPickupSpellAction(spell, true);
          ImGui::EndMenu();
        }
        if (!visible.onGround.empty())
        {
          const bool noRoomForItems = std::none_of(begin(visible.onGround), end(visible.onGround),
                                                   [&hero = state.hero](Item item) { return hero.hasRoomFor(item); });
          if (noRoomForItems)
          {
            if (!cannotTakePotion)
              ImGui::TextColored(colorUnavailable, "No room for items");
          }
          else if (ImGui::BeginMenu("Items"))
          {
            auto items = visible.onGround;
            for (Item item : items)
            {
              if (state.hero.hasRoomFor(item))
                addPickupItemAction(item);
            }
            ImGui::EndMenu();
          }
        }
        if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
          ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
      }
    }
  }

  void Arena::runMiscActions(const State& state)
  {
    const auto& visible = state.resources.visible;
    if (visible.numPlants > 0)
    {
      ImGui::SameLine();
      addActionButton(
          state, "Destroy plant",
          [](State& state) {
            state.hero.plantDestroyed(true);
            --state.resources.visible.numPlants;
            return Summary::None;
          },
          result);

      if (state.hero.has(Spell::Imawal) && Magic::isPossible(state.hero, Spell::Imawal, state.resources))
      {
        ImGui::SameLine();
        addActionButton(
            state, "Petrify plant",
            [](State& state) {
              state.hero.petrifyPlant(state.monsterPool);
              --state.resources.visible.numPlants;
              ++state.resources.visible.numWalls;
              return Summary::None;
            },
            result);
      }
    }

    if (visible.numBloodPools > 0 && state.hero.has(HeroStatus::Sanguine))
    {
      ImGui::SameLine();
      addActionButton(
          state, "Consume blood pool",
          [](State& state) {
            if (state.hero.bloodPoolConsumed())
              --state.resources.visible.numBloodPools;
            return Summary::None;
          },
          result);
    }
  }

  ActionResultUI Arena::run(const State& state)
  {
    result.reset();

    ImGui::Begin("Arena");
    ImGui::SetWindowPos(ImVec2{515, 5}, ImGuiCond_FirstUseEver);
    ImGui::SetWindowSize(ImVec2{500, 400}, ImGuiCond_FirstUseEver);
    showStatus(state);
    if (!state.hero.isDefeated())
    {
      runAttack(state);
      ImGui::SameLine();
      runCastPopup(state);
      ImGui::SameLine();
      runUseItemPopup(state);
      ImGui::SameLine();
      runConvertItemPopup(state);

      runShopPopup(state);
      ImGui::SameLine();
      runFaithPopup(state);
      ImGui::SameLine();
      runUncoverTiles(state);
      ImGui::SameLine();
      runPickupResource(state);
      runMiscActions(state);
    }
    ImGui::End();

    return result;
  }
} // namespace ui
