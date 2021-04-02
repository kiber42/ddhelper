#include "Arena.hpp"

#include "Utils.hpp"

#include "Combat.hpp"
#include "Hero.hpp"
#include "Items.hpp"
#include "Monster.hpp"
#include "MonsterTypes.hpp"
#include "Spells.hpp"

#include "imgui.h"

using namespace std::string_literals;

namespace
{
  void showPredictedOutcomeTooltip(const State& initialState, const GameAction& stateUpdate)
  {
    const auto result = applyAction(initialState, stateUpdate, true);
    const auto& newState = result.first;
    const auto& outcome = result.second;
    if (outcome.summary == Summary::None)
      return;
    createToolTip([&] {
      const auto outcomeStr = toString(outcome);
      if (!outcomeStr.empty())
        ImGui::TextColored(outcomeColor(outcome), "%s", outcomeStr.c_str());
      showStatus(newState);
    });
  }
} // namespace

void Arena::addAction(const State& state, std::string title, const GameAction& action, bool activated)
{
  if (activated)
    result.emplace(std::pair{std::move(title), std::move(action)});
  else if (ImGui::IsItemHovered())
    showPredictedOutcomeTooltip(state, action);
}

void Arena::addActionButton(const State& state, std::string title, const GameAction& action)
{
  const bool buttonPressed = ImGui::Button(title.c_str());
  addAction(state, std::move(title), action, buttonPressed);
}

bool Arena::addPopupAction(
    const State& state, std::string itemLabel, std::string historyTitle, const GameAction& action, bool wasSelected)
{
  const bool mouseDown = ImGui::IsAnyMouseDown();
  const bool becameSelected = (ImGui::Selectable(itemLabel.c_str()) || (ImGui::IsItemHovered() && mouseDown));
  addAction(state, std::move(historyTitle), action, wasSelected && !mouseDown);
  return becameSelected;
}

void Arena::runAttack(const State& state)
{
  const Monster* activeMonster = state.activeMonster >= 0 ? (&state.monsterPool[state.activeMonster]) : nullptr;
  if (activeMonster && !activeMonster->isDefeated())
  {
    addActionButton(state, "Attack",
                    [](State& state) { return Combat::attack(state.hero, *state.monster(), state.monsterPool); });
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
      const int costs = Magic::spellCosts(spell, state.hero);
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
      if (addPopupAction(state, label, historyTitle, cast, isSelected))
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
                isSelected))
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
                isSelected))
          selectedPopupItem = index;
      }
      else
        ImGui::TextColored(colorUnavailable, "%s", label.c_str());
    }

    const auto& inventory = state.hero.getItemsAndSpells();
    const bool canCompress =
        state.hero.has(Item::CompressionSeal) &&
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
                isSelected))
          selectedPopupItem = index;
      }
    }

    const bool canTransmute = state.hero.has(Item::TransmutationSeal) &&
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
        if (item == Item::TransmutationSeal && count == 1)
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
                isSelected))
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
          const std::string label = toString(item) + " ("s + std::to_string(itemEntry.conversionPoints) + " CP)";
          const std::string historyTitle = "Convert " + label;
          if (addPopupAction(
                  state, label, historyTitle,
                  [item](State& state) {
                    state.hero.convert(item, state.monsterPool);
                    return Summary::None;
                  },
                  isSelected))
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
                  isSelected))
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
  if (shops.empty())
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
    bool havePotionShop = false;
    if (state.hero.has(Item::TranslocationSeal))
      ImGui::Checkbox("Use Translocation Seal", &useTranslocationSeal);
    else
      useTranslocationSeal = false;
    for (size_t itemIndex = 0u; itemIndex < shops.size(); ++itemIndex)
    {
      const auto item = shops[itemIndex];
      if (item == Item::AnyPotion)
      {
        havePotionShop = true;
        continue;
      }
      const bool isSelected = ++index == selectedPopupItem;
      if (useTranslocationSeal)
      {
        const std::string label = toString(item) + " ("s + std::to_string(price(item)) + " gold)";
        const std::string historyTitle = "Translocate " + label;
        if (addPopupAction(
                state, label, historyTitle,
                [item, itemIndex](State& state) {
                  if (state.hero.useTranslocationSealOn(item))
                  {
                    auto& shops = state.resources().shops;
                    shops.erase(begin(shops) + itemIndex);
                  }
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      else
      {
        const int price = state.hero.buyingPrice(item);
        const std::string label = toString(item) + " ("s + std::to_string(price) + " gold)";
        if (state.hero.gold() >= price)
        {
          const std::string historyTitle = "Buy " + label;
          if (addPopupAction(
                  state, label, historyTitle,
                  [item, itemIndex](State& state) {
                    if (state.hero.buy(item))
                    {
                      auto& shops = state.resources().shops;
                      shops.erase(begin(shops) + itemIndex);
                    }
                    return Summary::None;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextColored(colorUnavailable, "%s", label.c_str());
      }
    }
    if (havePotionShop && ImGui::BeginMenu("Potion Shop"))
    {
      for (int potionIndex = static_cast<int>(Item::HealthPotion); potionIndex <= static_cast<int>(Item::CanOfWhupaz);
           ++potionIndex)
      {
        const bool isSelected = ++index == selectedPopupItem;
        const auto potion = static_cast<Item>(potionIndex);
        const int price = state.hero.buyingPrice(potion);
        const std::string label = toString(potion) + " ("s + std::to_string(price) + " gold)";
        const std::string historyTitle = "Buy " + label;
        if (state.hero.gold() >= price)
        {
          if (addPopupAction(
                  state, label, historyTitle,
                  [potion](State& state) {
                    if (state.hero.buy(potion))
                    {
                      auto& shops = state.resources().shops;
                      shops.erase(std::find(begin(shops), end(shops), Item::AnyPotion));
                    }
                    return Summary::None;
                  },
                  isSelected))
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
  const auto& altars = state.resources().altars;
  if (altars.empty() && (!state.resources().pactMakerAvailable || state.hero.getFaith().getPact().has_value()))
  {
    disabledButton("Faith", "No altars");
    return;
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
                  isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextColored(colorUnavailable, "%s", label.c_str());
      }
    }
    const bool haveAvailableAltars =
        !altars.empty() &&
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
                    return state.hero.getFaith().followDeity(deity, state.hero) ? Summary::None : Summary::NotPossible;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        else
          ImGui::TextColored(colorUnavailable, "%s", label.c_str());
      }
    }

    if (state.resources().pactMakerAvailable && !state.hero.getFaith().getPact())
    {
      if (following || haveAvailableAltars)
        ImGui::Separator();
      ImGui::TextUnformatted("Pactmaker");
      ImGui::Separator();
      const int last =
          static_cast<int>(state.hero.getFaith().enteredConsensus() ? Pact::LastNoConsensus : Pact::LastWithConsensus);
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
                isSelected))
          selectedPopupItem = index;
      }
    }

    if (following && haveAvailableAltars)
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
                [god, altarIndex](State& state) {
                  state.hero.desecrate(god, state.monsterPool);
                  auto& altars = state.resources().altars;
                  altars.erase(begin(altars) + altarIndex);
                  return Summary::Safe;
                },
                isSelected))
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
  auto uncoverForAll = [](State& state, int numSquares) {
    state.hero.recover(numSquares);
    for (auto& monster : state.monsterPool)
      monster.recover(numSquares);
    return Summary::None;
  };

  addActionButton(state, "Uncover Tile", [&](State& state) { return uncoverForAll(state, 1); });

  const int numSquares = state.hero.numSquaresForFullRecovery();
  if (numSquares > 1)
  {
    ImGui::SameLine();
    const std::string label = "Uncover " + std::to_string(numSquares) + " Tiles";
    addActionButton(state, label, [&](State& state) { return uncoverForAll(state, numSquares); });
  }
}

void Arena::runFindPopup(const State& state)
{
  ImGui::Button("Find");
  if (ImGui::IsItemActive())
  {
    ImGui::OpenPopup("FindPopup");
    selectedPopupItem = -1;
  }
  if (ImGui::BeginPopup("FindPopup"))
  {
    int index = 0;
    if (ImGui::BeginMenu("Spells"))
    {
      for (int spellIndex = 0; spellIndex <= static_cast<int>(Spell::Last); ++spellIndex)
      {
        const Spell spell = static_cast<Spell>(spellIndex);
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction(
                state, toString(spell), "Find "s + toString(spell),
                [spell](State& state) {
                  state.hero.receive(spell);
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Altars"))
    {
      for (int altarIndex = 0; altarIndex <= static_cast<int>(God::Last); ++altarIndex)
      {
        const God god = static_cast<God>(altarIndex);
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction(
                state, toString(god), "Find "s + toString(god) + "'s altar",
                [god](State& state) {
                  state.resources().altars.emplace_back(god);
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      if (!state.resources().pactMakerAvailable)
      {
        const bool isSelected = ++index == selectedPopupItem;
        if (addPopupAction(
                state, "The Pactmaker", "Find The Pactmaker's altar",
                [](State& state) {
                  state.resources().pactMakerAvailable = true;
                  return Summary::None;
                },
                isSelected))
          selectedPopupItem = index;
      }
      ImGui::EndMenu();
    }

    const int isSelected = ++index == selectedPopupItem;
    if (addPopupAction(
            state, "Add potion shop", "Add potion shop",
            [](State& state) {
              state.resources().shops.emplace_back(Item::AnyPotion);
              return Summary::None;
            },
            isSelected))
      selectedPopupItem = index;

    struct SubMenu
    {
      std::string title;
      Item first;
      Item last;
    };
    const std::vector<SubMenu> submenus = {{"Blacksmith Items", Item::BearMace, Item::Sword},
                                           {"Basic Items", Item::BadgeOfHonour, Item::TrollHeart},
                                           {"Quest Items", Item::PiercingWand, Item::SoulOrb},
                                           {"Elite Items", Item::KegOfHealth, Item::WickedGuitar},
                                           {"Boss Rewards", Item::FabulousTreasure, Item::SensationStone}};
    for (auto submenu : submenus)
    {
      if (ImGui::BeginMenu(submenu.title.c_str()))
      {
        for (int itemIndex = static_cast<int>(submenu.first); itemIndex <= static_cast<int>(submenu.last); ++itemIndex)
        {
          const bool isSelected = ++index == selectedPopupItem;
          const auto item = static_cast<Item>(itemIndex);
          if (addPopupAction(
                  state, toString(item), "Add shop: "s + toString(item),
                  [item](State& state) {
                    state.resources().shops.emplace_back(item);
                    return Summary::None;
                  },
                  isSelected))
            selectedPopupItem = index;
        }
        ImGui::EndMenu();
      }
    }
    if (ImGui::BeginMenu("Cheat"))
    {
      if (addPopupAction(
              state, "+50 piety", "Cheat: +50 piety",
              [](State& state) {
                state.hero.getFaith().gainPiety(50);
                return Summary::None;
              },
              ++index == selectedPopupItem))
        selectedPopupItem = index;
      if (addPopupAction(
              state, "+20 gold", "Cheat: +20 gold",
              [](State& state) {
                state.hero.addGold(20);
                return Summary::None;
              },
              ++index == selectedPopupItem))
        selectedPopupItem = index;
      ImGui::EndMenu();
    }
    if (!ImGui::IsAnyMouseDown() && selectedPopupItem != -1)
      ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
  }
}

Arena::ArenaResult Arena::run(const State& state)
{
  result.reset();

  ImGui::Begin("Arena");
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

    runFindPopup(state);
  }
  ImGui::End();

  return result;
}
