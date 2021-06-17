#include "bandit/bandit.h"

#include "engine/DungeonSetup.hpp"
#include "engine/GodsAndBoons.hpp"
#include "engine/Resources.hpp"
#include "engine/Spells.hpp"

#include <random>
#include <set>

using namespace bandit;
using namespace snowhouse;

void testResources()
{
  describe("Default resources", [] {
    ResourceSet resourceSet{DungeonSetup{}};
    it("shall be initialized randomly", [&] {
      ResourceSet resourceSet2{DungeonSetup{}};
      AssertThat(resourceSet, !Equals(resourceSet2));
    });
    it("shall have 3 different altars", [&altars = resourceSet.altars] {
      AssertThat(altars.size(), Equals(3u));
      AssertThat(std::set(begin(altars), end(altars)).size(), Equals(3u));
    });
    it("shall have 5 different spells", [&spells = resourceSet.spells] {
      AssertThat(spells.size(), Equals(5u));
      AssertThat(std::set(begin(spells), end(spells)).size(), Equals(5u));
    });
    it("shall have 8 different shops", [&shops = resourceSet.shops] {
      AssertThat(shops.size(), Equals(8u));
      AssertThat(std::set(begin(shops), end(shops)).size(), Equals(8u));
    });
    it("shall have Burndayraz", [&spells = resourceSet.spells] { AssertThat(spells, Contains(Spell::Burndayraz)); });
    it("shall avoid duplicate resources", [] {
      std::mt19937 generator{std::random_device{}()};
      ResourceSet lotsOf;
      for (int i = 0; i < 100; ++i)
      {
        lotsOf.addRandomAltar(generator);
        lotsOf.addRandomSpell(generator);
        lotsOf.addRandomShop(generator);
      }
      AssertThat(lotsOf.altars.size(), Equals((unsigned)God::Last + 2));
      AssertThat(lotsOf.spells.size(), Equals((unsigned)Spell::Last + 1));
      AssertThat(lotsOf.shops.size(), IsLessThanOrEqualTo(100u));
    });
  });

  describe("Custom dungeon setups", [] {
    it("shall respect hero class traits", [] {
      ResourceSet hoarder{DungeonSetup{HeroClass::Thief, HeroRace::Human}};
      AssertThat(hoarder.numGoldPiles, Equals(13u));
      AssertThat(hoarder.shops.size(), Equals(10u));
      ResourceSet martyr{DungeonSetup{HeroClass::Crusader, HeroRace::Human}};
      AssertThat(martyr.altars.size(), Equals(4u));
      ResourceSet merchant{
          DungeonSetup{HeroClass::Tinker, HeroRace::Goblin, {}, {}, {}, {}, {God::TikkiTooki}, {}, {}, {}}};
      AssertThat(merchant.numGoldPiles, Equals(10u));
      AssertThat(merchant.shops.size(), Equals(10u));
      AssertThat(merchant.altars, Contains(GodOrPactmaker{God::TikkiTooki}));
    });
    it("shall accept modifiers for dungeon preparations", [] {
      DungeonSetup setup;
      setup.modifiers = {MageModifier::ExtraManaBoosters, MageModifier::FlameMagnet, MageModifier::ExtraGlyph,
                         BazaarModifier::Apothecary};
      ResourceSet prepared{setup};
      AssertThat(prepared.numAttackBoosters, Equals(3u));
      AssertThat(prepared.numManaBoosters, Equals(5u));
      AssertThat(prepared.numHealthBoosters, Equals(3u));
      AssertThat(prepared.spells, !Contains(Spell::Burndayraz));
      AssertThat(prepared.spells.size(), Equals(5u));
      AssertThat(prepared.numPotionShops, Equals(3u));
      setup.modifiers = {MageModifier::ExtraHealthBoosters, MageModifier::FewerGlyphs};
      setup.altar = Pactmaker::ThePactmaker;
      ResourceSet prepared2{setup};
      AssertThat(prepared2.numManaBoosters, Equals(3u));
      AssertThat(prepared2.numHealthBoosters, Equals(5u));
      AssertThat(prepared2.spells, Contains(Spell::Burndayraz));
      AssertThat(prepared2.spells.size(), Equals(4u));
      AssertThat(prepared2.numPotionShops, Equals(1u));
      AssertThat(prepared2.altars.size(), Equals(4u));
    });
  });

  describe("Simple Resources", [] {
    it("shall have a pretend reveal mechanism", [] {
      SimpleResources resources{ResourceSet{DungeonSetup{}}};
      auto initialResourceSet = resources();
      const auto initialTiles = resources.numHiddenTiles;
      AssertThat(initialTiles, IsGreaterThan(100u));
      resources.revealTile();
      AssertThat(resources.numHiddenTiles, Equals(initialTiles - 1u));
      resources.revealTiles(50);
      AssertThat(resources.numHiddenTiles, Equals(initialTiles - 51u));
      AssertThat(resources(), Equals(initialResourceSet));
      resources.revealTiles(resources.numHiddenTiles);
      AssertThat(resources.numHiddenTiles, Equals(0u));
      AssertThat(resources(), Equals(initialResourceSet));
    });
    it("shall have a constructor that allows to define the available resources", [] {
      ResourceSet resourceSet{DungeonSetup{}};
      SimpleResources resources{resourceSet};
      AssertThat(resources(), Equals(resourceSet));
    });
  });

  describe("Map Resources", [] {
    it("shall distinguish between visible and hidden resources", [] {
      MapResources resources;
      AssertThat(resources().numGoldPiles, IsLessThan(10u));
    });
    it("shall eventually reveal all resources", [] {
      MapResources resources;
      resources.revealTiles(resources.numHiddenTiles);
      AssertThat(resources().numGoldPiles, Equals(10u));
    });
    it("shall have a constructor that allows to define the available resources", [] {
      ResourceSet empty;
      ResourceSet resourceSet{DungeonSetup{}};
      MapResources initiallyVisible{resourceSet, empty};
      AssertThat(initiallyVisible(), Equals(resourceSet));
      initiallyVisible.revealTiles(initiallyVisible.numHiddenTiles);
      AssertThat(initiallyVisible(), Equals(resourceSet));

      MapResources initiallyHidden{empty, resourceSet, 0};
      initiallyHidden.numHiddenTiles = 400;
      AssertThat(initiallyHidden(), Equals(empty));
      initiallyHidden.revealTiles(initiallyHidden.numHiddenTiles);
      [[maybe_unused]] auto sortResources = [](ResourceSet& resourceSet) {
        std::sort(begin(resourceSet.altars), end(resourceSet.altars));
        std::sort(begin(resourceSet.spells), end(resourceSet.spells));
        std::sort(begin(resourceSet.shops), end(resourceSet.shops));
      };
      sortResources(resourceSet);
      sortResources(initiallyHidden());
      AssertThat(initiallyHidden(), Equals(resourceSet));
    });
  });
}
