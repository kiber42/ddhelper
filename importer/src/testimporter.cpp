#include "bandit/bandit.h"

#include "importer/testimporter.hpp"
#include "importer/ImageProcessor.hpp"
#include "importer/ImportedState.hpp"

#include "engine/MonsterTypes.hpp"
#include "engine/StrongTypes.hpp"

using namespace bandit;
using namespace snowhouse;

namespace snowhouse
{
  template <>
  struct Stringizer<MonsterType>
  {
    static std::string ToString(const MonsterType& monsterType) { return toString(monsterType); }
  };
}

go_bandit([] {
  describe("Importer", [] {
    auto testLevels = [&](const auto monsterInfos, const std::vector<int> expected) {
      AssertThat(monsterInfos.size(), Equals(expected.size()));
      std::vector<int> levels(monsterInfos.size());
      std::transform(begin(monsterInfos), end(monsterInfos), begin(levels),
                     [](const auto& info) { return info.level.get(); });
      AssertThat(levels, Equals(expected));
    };

    it("shall identify monster levels in reference image (Linux)", [&] {
      testLevels(importer::ImageProcessor::findMonstersInScreenshotLinux(resourceDir + "ref01.png"),
        {1, 9, 6, 9, 2, 8, 1, 3, 1, 2, 7, 2, 1, 8, 5, 5, 10, 5, 1, 8});
    });
    it("shall identify monster levels in reference image (Windows)", [&] {
      testLevels(importer::ImageProcessor::findMonstersInScreenshotWindows(resourceDir + "ref01_win.png"),
                 {2, 1, 1, 6, 5, 9, 4, 7, 8, 8, 9, 10, 6, 2, 2, 6, 7, 7, 1,
                  4, 5, 1, 5, 1, 2, 3, 3, 5, 3, 2, 1,  4, 4, 8, 1, 1, 1, 3});
    });

    auto testMonsterTypes = [&](const auto monsterInfos, const std::vector<MonsterType> expected) {
      AssertThat(monsterInfos.size(), Equals(expected.size()));
      std::vector<MonsterType> types(monsterInfos.size());
      std::transform(begin(monsterInfos), end(monsterInfos), begin(types), [](const auto& info) { return info.type; });
      AssertThat(types, Equals(expected));
    };

    it("shall identify monster types in reference image (Linux)", [&] {
      testMonsterTypes(importer::ImageProcessor::findMonstersInScreenshotLinux(resourceDir + "ref02.png"),
                       {MonsterType::Goblin,  MonsterType::Goat,    MonsterType::DragonSpawn, MonsterType::Goblin,
                        MonsterType::Serpent, MonsterType::Zombie,  MonsterType::Gorgon,      MonsterType::Zombie,
                        MonsterType::Gorgon,  MonsterType::Gorgon,  MonsterType::Goblin,      MonsterType::Wraith,
                        MonsterType::Bandit,  MonsterType::GooBlob, MonsterType::Goblin,      MonsterType::Warlock,
                        MonsterType::Serpent, MonsterType::Golem,   MonsterType::MeatMan,     MonsterType::MeatMan,
                        MonsterType::Goblin,  MonsterType::Zombie,  MonsterType::Goblin,      MonsterType::Gorgon,
                        MonsterType::Serpent, MonsterType::Gorgon,  MonsterType::DragonSpawn, MonsterType::Gorgon,
                        MonsterType::MeatMan, MonsterType::Goblin,  MonsterType::Wraith,      MonsterType::Warlock,
                        MonsterType::Zombie,  MonsterType::GooBlob, MonsterType::GooBlob,     MonsterType::Warlock,
                        MonsterType::MeatMan, MonsterType::Golem,   MonsterType::Bandit});
    });
    it("shall identify monster types in reference image (Windows)", [&] {
      testMonsterTypes(
          importer::ImageProcessor::findMonstersInScreenshotWindows(resourceDir + "ref02_win.png"),
          {MonsterType::Warlock, MonsterType::Vampire, MonsterType::Vampire, MonsterType::Warlock, MonsterType::MeatMan,
           MonsterType::Warlock, MonsterType::Warlock, MonsterType::Vampire, MonsterType::MeatMan, MonsterType::Goblin,
           MonsterType::MeatMan, MonsterType::Goblin,  MonsterType::Warlock, MonsterType::Vampire, MonsterType::MeatMan,
           MonsterType::Vampire, MonsterType::MeatMan, MonsterType::MeatMan, MonsterType::Warlock, MonsterType::MeatMan,
           MonsterType::Zombie,  MonsterType::Warlock, MonsterType::Vampire, MonsterType::Zombie,  MonsterType::Zombie,
           MonsterType::Zombie,  MonsterType::Warlock, MonsterType::Warlock, MonsterType::Zombie});
    });
  });
});

int main(int argc, char* argv[])
{
  return bandit::run(argc, argv);
}
