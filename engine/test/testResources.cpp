#include "bandit/bandit.h"

#include "Resources.hpp"
#include "Spells.hpp"

#include <set>

using namespace bandit;
using namespace snowhouse;

void testResources()
{
  describe("Default resources", [] {
    ResourceSet resourceSet{DefaultResources{}};
    it("shall be initialized randomly", [&] {
      ResourceSet resourceSet2{DefaultResources{}};
      AssertThat(resourceSet, !Equals(resourceSet2));
    });
    it("shall have 3 different altars",
       [pactMaker = (unsigned)resourceSet.pactMakerAvailable, &altars = resourceSet.altars] {
         AssertThat(altars.size() + pactMaker, Equals(3u));
         AssertThat(std::set(begin(altars), end(altars)).size() + pactMaker, Equals(3u));
       });
    it("shall have 5 different spells", [&spells = resourceSet.spells] {
      AssertThat(spells.size(), Equals(5u));
      AssertThat(std::set(begin(spells), end(spells)).size(), Equals(5u));
    });
    it("shall have Burndayraz", [&spells = resourceSet.spells] {
      AssertThat(std::find(begin(spells), end(spells), Spell::Burndayraz) != end(spells), IsTrue());
    });
  });

  describe("Simple Resources", [] {
    it("shall have a pretend reveal mechanism", [] {
      SimpleResources resources;
      auto initialResourceSet = resources();
      const int initialTiles = resources.numHiddenTiles;
      AssertThat(initialTiles, IsGreaterThan(100));
      resources.revealTile();
      AssertThat(resources.numHiddenTiles, Equals(initialTiles - 1));
      resources.revealTiles(50);
      AssertThat(resources.numHiddenTiles, Equals(initialTiles - 51));
      AssertThat(resources(), Equals(initialResourceSet));
      resources.revealTiles(resources.numHiddenTiles);
      AssertThat(resources.numHiddenTiles, Equals(0));
      AssertThat(resources(), Equals(initialResourceSet));
    });
    it("shall have a constructor that allows to define the available resources", [] {
      ResourceSet resourceSet{DefaultResources{}};
      SimpleResources resources{resourceSet};
      AssertThat(resources(), Equals(resourceSet));
    });
  });

  describe("Map Resources", [] {
    it("shall distinguish between visible and hidden resources", [] {
      MapResources resources;
      AssertThat(resources().numGoldPiles, IsLessThan(10));
    });
    it("shall eventually reveal all resources", [] {
      MapResources resources;
      resources.revealTiles(resources.numHiddenTiles);
      AssertThat(resources().numGoldPiles, Equals(10));
    });
    it("shall have a constructor that allows to define the available resources", [] {
      ResourceSet empty{EmptyResources{}};
      ResourceSet resourceSet{DefaultResources{}};
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
