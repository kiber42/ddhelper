// Include OpenCV before any X11 headers
#include <opencv2/opencv.hpp>

#include "importer/GameWindow.hpp"
#include "importer/ImageCapture.hpp"
#include "importer/ImageProcessor.hpp"

#include "engine/Monster.hpp"
#include "engine/MonsterTypes.hpp"

#include <iomanip>
#include <string>
#include <tuple>
#include <utility>

namespace
{
  std::string to_hex(int value)
  {
    // C++20 std::format not available in most compilers
    // return std::format("{:x}", value);
    std::stringstream sstr;
    sstr << std::hex << value;
    return sstr.str();
  }

  std::string describe(const importer::MonsterInfo& info, DungeonMultiplier multiplier)
  {
    const bool isGeneric = info.type == MonsterType::Generic;
    using namespace std::string_literals;
    std::string name = toString(info.type) + " level "s + std::to_string(info.level.get());
    if (isGeneric)
      name += " (hash: 0x"s + to_hex(info.hash) + ')';
    const std::string position = "\tat " + std::to_string(info.position.x) + ", " + std::to_string(info.position.y);
    const auto referenceMonster = Monster{info.type, info.level, multiplier};
    const auto referenceHitPoints = referenceMonster.getHitPointsMax();
    std::string hitpoints;
    if (info.health)
    {
      hitpoints = "\tHP: " + std::to_string(info.health->first) + '/' + std::to_string(info.health->second);
      if (referenceHitPoints != static_cast<unsigned>(info.health->second))
        hitpoints += " (unexpected! reference value is " + std::to_string(referenceMonster.getHitPointsMax()) + ')';
    }
    else {
      if (!isGeneric)
        hitpoints = "\treference HP: " + std::to_string(referenceMonster.getHitPointsMax());
      if (info.hasHealthBar)
        hitpoints += " [has health bar]";
    }

    return name + position + hitpoints;
  }

  void describe(const importer::ImportedState& state, DungeonMultiplier multiplier)
  {
    for (auto& info : state.monsterInfos)
    {
      std::cout << describe(info, multiplier) << std::endl;
    }
  }

  bool processImagesFromGameWindow(DungeonMultiplier multiplier, bool runcheck)
  {
    importer::GameWindow gameWindow;
    if (!gameWindow.valid())
    {
      std::cerr << "Could not find game window." << std::endl;
      return false;
    }
    importer::ImageCapture capture(gameWindow);
    unsigned numFrames = 0;
    std::size_t size = 0;
    while (true)
    {
      importer::ImageProcessor processor(capture);
      try
      {
        processor.findMonsters(5, 110);
      }
      catch (const std::runtime_error& e)
      {
        std::cerr << e.what() << std::endl;
        if (!capture.current())
          break;
        cv::waitKey(500);
      }
      if (runcheck)
        processor.extractMonsterInfos(false);
      const auto& state = processor.get();
      if (runcheck || state.monsterInfos.size() != size)
      {
        std::cout << std::string(80, '*') << std::endl;
        describe(state, multiplier);
        size = state.monsterInfos.size();
        cv::waitKey(2000);
      }
      else
        cv::waitKey(100);
      ++numFrames;
    }
    return numFrames > 0;
  }

  bool processImageFromFile(std::filesystem::path path, DungeonMultiplier multiplier)
  {
    auto monsterInfos = importer::ImageProcessor::findMonstersInScreenshot(path);
    describe({std::move(monsterInfos)}, multiplier);
    return true;
  }
} // namespace

auto parseArgs(int argc, char** argv)
{
  auto multiplier = DungeonMultiplier{1};
  std::string path;
  if (argc > 1)
  {
    auto value = atof(argv[1]);
    if (value > 0)
    {
      multiplier = DungeonMultiplier{value < 10 ? value : value / 100};
      if (argc > 2)
        path = argv[2];
    }
    else
      path = argv[1];
  }
  bool runcheck = true;
  if (path == "nocheck")
  {
    path.clear();
    runcheck = false;
  }
  return std::tuple{multiplier, std::move(path), runcheck};
}

int main(int argc, char** argv)
{
  const auto [multiplier, path, runcheck] = parseArgs(argc, argv);
  const bool success =
      path.empty() ? processImagesFromGameWindow(multiplier, runcheck) : processImageFromFile(path, multiplier);
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
