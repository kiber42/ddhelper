#include "bandit/bandit.h"

#include "importer/testimporter.hpp"
#include "importer/ImageProcessor.hpp"
#include "importer/ImportedState.hpp"

#include "engine/MonsterTypes.hpp"
#include "engine/StrongTypes.hpp"

using namespace bandit;
using namespace snowhouse;

go_bandit([] {
  describe("Importer", [] {
    it("shall identify monster levels in reference image", [] {
      const auto monsterInfos = importer::ImageProcessor::findMonstersInScreenshot(resourceDir + "ref01.png");
      AssertThat(monsterInfos.size(), Equals(20u));
      std::vector<int> levels(monsterInfos.size());
      std::transform(begin(monsterInfos), end(monsterInfos), begin(levels),
                     [](const auto& info) { return info.level.get(); });
      const std::vector expected = {1, 9, 6, 9, 2, 8, 1, 3, 1, 2, 7, 2, 1, 8, 5, 5, 10, 5, 1, 8};
      AssertThat(levels, Equals(expected));
    });
    it("shall identify monster types in reference image", [] {
      const auto monsterInfos = importer::ImageProcessor::findMonstersInScreenshot(resourceDir + "ref02.png");
      AssertThat(monsterInfos.size(), Equals(39u));
      std::vector<MonsterType> types(monsterInfos.size());
      std::transform(begin(monsterInfos), end(monsterInfos), begin(types), [](const auto& info) { return info.type; });
      const std::vector expected = {
          MonsterType::Goblin,  MonsterType::Goat,    MonsterType::DragonSpawn, MonsterType::Goblin,
          MonsterType::Serpent, MonsterType::Zombie,  MonsterType::Gorgon,      MonsterType::Zombie,
          MonsterType::Gorgon,  MonsterType::Gorgon,  MonsterType::Goblin,      MonsterType::Wraith,
          MonsterType::Bandit,  MonsterType::GooBlob, MonsterType::Goblin,      MonsterType::Warlock,
          MonsterType::Serpent, MonsterType::Golem,   MonsterType::MeatMan,     MonsterType::MeatMan,
          MonsterType::Goblin,  MonsterType::Zombie,  MonsterType::Goblin,      MonsterType::Gorgon,
          MonsterType::Serpent, MonsterType::Gorgon,  MonsterType::DragonSpawn, MonsterType::Gorgon,
          MonsterType::MeatMan, MonsterType::Goblin,  MonsterType::Wraith,      MonsterType::Warlock,
          MonsterType::Zombie,  MonsterType::GooBlob, MonsterType::GooBlob,     MonsterType::Warlock,
          MonsterType::MeatMan, MonsterType::Golem,   MonsterType::Bandit};
      AssertThat(types, Equals(expected));
    });
  });
});

int main(int argc, char* argv[])
{
  return bandit::run(argc, argv);
}
