#include "importer/ImageProcessor.hpp"
#include "importer/GameWindow.hpp"
#include "importer/ImageCapture.hpp"
#include "importer/PixelAdapters.hpp"

#include <opencv2/opencv.hpp>

#include "importer/Mouse.hpp"

#include <thread>

namespace importer
{
  namespace
  {
    // On Linux/X11, the window contents are shifted one pixel up: the topmost row is missing, and a black row of pixels
    // appears at the bottom of the window
    constexpr int y_offset = -1;

    constexpr int required_screen_size_x = 800;
    constexpr int required_screen_size_y = 600;

    // Identify monster levels using the red and green components of 3 specific pixels on a tile.
    // Pixel positions are (3, 18), (4, 22) (3, 24)
    static constexpr std::array<uint64_t, 10> levelFromPixels = {
        0x00FF00FF0000, 0x00FF000000FF, 0x000000FF0000, 0xFFFF0000FFFF, 0xFFFFFFFF0000,
        0xFFFFFFFFFFFF, 0x0000FF800000, 0xFF80FF80FF80, 0xFF80FF800000, 0xFF00FF000000,
    };

    template <typename PixelFunc>
    std::optional<Level> getMonsterLevel(const cv::Mat& dungeon, int x, int y)
    {
      // Level number (if any) is a 16x16 sprite overlayed in the lower left corner of a 30x30 tile.
      const int start_x = x * 30;
      const int start_y = y * 30 + y_offset;
      // Several suitable combinations of 3 pixels exist that suffice to tell them apart, we use (3, 18), (4, 22) (3,
      // 24).
      const auto pixel1 = PixelFunc(dungeon, Column{start_x + 3}, Row{start_y + 18});
      const auto pixel2 = PixelFunc(dungeon, Column{start_x + 4}, Row{start_y + 22});
      const auto pixel3 = PixelFunc(dungeon, Column{start_x + 3}, Row{start_y + 24});
      // Level labels have no blue component
      if (pixel1.b() == 0 && pixel2.b() == 0 && pixel3.b() == 0)
      {
        const uint64_t hash = (static_cast<uint64_t>(pixel1.rg()) << 32) + (static_cast<uint64_t>(pixel2.rg()) << 16) +
                              static_cast<uint64_t>(pixel3.rg());
        const auto iter = std::find(begin(levelFromPixels), end(levelFromPixels), hash);
        if (iter != end(levelFromPixels))
        {
          const auto index = std::distance(begin(levelFromPixels), iter);
          return Level{index + 1};
        }
      }
      return {};
    }

    // Identify monster by color of pixel at (11, 4)
    const std::map<uint32_t, MonsterType> monsterFromPixel = {
        // Basic Monsters
        {0xCC7F54, MonsterType::Bandit},
        {0x4BB56C, MonsterType::DragonSpawn},
        {0x544130, MonsterType::Goat},
        {0xE65918, MonsterType::Goblin},
        {0x7C6E57, MonsterType::Golem},
        {0x373E01, MonsterType::GooBlob},
        {0x0BA837, MonsterType::Gorgon},
        {0xFF6666, MonsterType::MeatMan},
        {0xBA863E, MonsterType::Serpent},
        {0xC74E73, MonsterType::Warlock},
        {0x090D3F, MonsterType::Wraith},
        {0x378A85, MonsterType::Zombie},
        // Advanced Monsters
        {0x3E0A01, MonsterType::AcidBlob},
        {0xA2FBD5, MonsterType::Changeling},
        {0xA3FBD5, MonsterType::Changeling},
        {0xA3FCD5, MonsterType::Changeling},
        {0x664EC7, MonsterType::Cultist},
        {0x9B7036, MonsterType::DesertTroll},
        {0x1285FD, MonsterType::Djinn},
        {0xFF3222, MonsterType::DoomArmour},
        {0xB3FF66, MonsterType::GelatinousThing},
        {0x79341C, MonsterType::Minotaur},
        {0x718A3F, MonsterType::Naga},
        {0xB6AAA1, MonsterType::Ratling},
        {0x25013E, MonsterType::SlimeBlob},
        {0x57657C, MonsterType::SteelGolem},
        {0xA8860B, MonsterType::Succubus},
        {0xBB54CC, MonsterType::Thrall},
        {0xBBD6D5, MonsterType::Vampire},
    };

    template <typename PixelFunc>
    inline uint32_t getTileHash(const cv::Mat& tile, int x, int y)
    {
      return PixelFunc(tile, Column{x * 30 + 12}, Row{y * 30 + y_offset + 5}).rgb();
    }

    cv::Mat acquireValidScreenshot(ImageCapture& capture)
    {
      auto ximage = capture.acquire();
      if (!ximage)
        throw std::runtime_error("Failed to acquire image.");
      if (ximage->width != required_screen_size_x || ximage->height != required_screen_size_y)
      {
        throw std::runtime_error("Please resize game window to " + std::to_string(required_screen_size_x) + "x" +
                                 std::to_string(required_screen_size_y) + " (current size: " +
                                 std::to_string(ximage->width) + "x" + std::to_string(ximage->height) + ")");
      }
      return cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
    }

    // Find all monsters on map shown in the input image.
    // Returns info about monsters and a flag that indicates whether all monsters could be identified.
    template <class PixelClass>
    std::pair<std::vector<MonsterInfo>, bool> findMonstersImpl(const cv::Mat& image)
    {
      std::pair<std::vector<MonsterInfo>, bool> result;
      auto& [monsters, complete] = result;
      complete = true;
      for (unsigned index = 0u; index < 400u; ++index)
      {
        const unsigned char x = index % 20;
        const unsigned char y = static_cast<unsigned char>(index / 20);
        const auto monsterLevel = getMonsterLevel<PixelClass>(image, x, y);
        if (monsterLevel)
        {
          const auto hash = getTileHash<PixelClass>(image, x, y);
          auto monsterType = MonsterType::Generic;
          if (auto detected = monsterFromPixel.find(hash); detected != end(monsterFromPixel))
            monsterType = detected->second;
          else
            complete = false;
          monsters.emplace_back(MonsterInfo{TilePosition{x, y}, monsterType, *monsterLevel, {}, hash});
        }
      }
      return result;
    }

    // Identify monster HP and max HP by counting number of "bright" pixels per column
    constexpr std::array digitFromBrightPixels = {52225, 11900, 22224, 22324, 23391, 23323,
                                                  50333, 13221, 63336, 33305, 0};

    template <typename PixelFunc>
    std::optional<std::pair<int, int>> extract_hp_or_mp(const cv::Mat& imagePart)
    {
      auto countBrightPixels = [&, height = imagePart.size[0]](int x) {
        int count = 0;
        for (int y = 0; y < height; ++y)
        {
          if (PixelFunc(imagePart, Column{x}, Row{y}).g() >= 189)
            ++count;
        }
        return count;
      };
      auto parseNumbers = [&, x = 0, width = imagePart.size[1]]() mutable -> std::optional<int> {
        int current = 0;
        while (x + 8 < width)
        {
          const int nextPattern = 10000 * countBrightPixels(x) + 1000 * countBrightPixels(x + 1) +
                                  100 * countBrightPixels(x + 2) + 10 * countBrightPixels(x + 3) +
                                  countBrightPixels(x + 4);
          const auto match = std::find(begin(digitFromBrightPixels), end(digitFromBrightPixels), nextPattern);
          if (match == end(digitFromBrightPixels))
            return {};
          const auto index = static_cast<int>(std::distance(begin(digitFromBrightPixels), match));
          if (index == 10)
            break;
          current = 10 * current + index;
          // Proceed to next number (gap between adjacent numbers is two pixels wide)
          x += 7;
          // Detect if following character is a slash (sequence 01310; gaps before and after only one pixel wide)
          if (countBrightPixels(x - 1) == 1)
          {
            const bool isSlash = countBrightPixels(x) == 3 && countBrightPixels(x + 1) == 1;
            if (isSlash)
            {
              x += 3;
              break;
            }
            return {};
          }
        }
        return current;
      };
      auto points = parseNumbers();
      auto maxPoints = parseNumbers();
      if (points && maxPoints)
        return {{*points, *maxPoints}};
      return std::nullopt;
    }

    // Move mouse over given tile and extract additional information from sidebar
    bool extractMonsterInfoImpl(MonsterInfo& infoToUpdate, GameWindow& gameWindow, ImageCapture& capture)
    {
      moveMouseTo(gameWindow.getDisplay(), gameWindow.getWindow(), 30 * infoToUpdate.position.x + 15,
                  30 * infoToUpdate.position.y + 15);
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(100ms);
      if (auto ximage = capture.acquire())
      {
        auto image = cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
        auto hitpoints = extract_hp_or_mp<PixelARGB>(image(cv::Rect(651, 430, 100, 10)));
        if (hitpoints)
        {
          infoToUpdate.health = hitpoints;
          return true;
        }
      }
      return false;
    }
  } // namespace

  ImageProcessor::ImageProcessor(ImageCapture& capture)
    : capture(capture)
  {
  }

  bool ImageProcessor::findMonsters(int numRetries, int retryDelayInMilliseconds)
  {
    auto image = acquireValidScreenshot(capture);
    auto [infos, complete] = findMonstersImpl<PixelARGB>(image);
    state.monsterInfos = std::move(infos);
    while (!complete && numRetries-- > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds{retryDelayInMilliseconds});
      complete = retryFindMonsters();
    }
    return complete;
  }

  // Take screenshot and try to update information about previously unidentied monsters.
  // This might help if a monster could not previously be identified due to an animation (monster is slowed, or
  // there's a piety token on the same square).
  // Returns true if no unidentified / generic monsters remain.
  bool ImageProcessor::retryFindMonsters()
  {
    auto image = acquireValidScreenshot(capture);

    bool complete = true;
    auto isGenericMonster = [](const auto& monster) { return monster.type == MonsterType::Generic; };
    auto nextUnknown = std::find_if(begin(state.monsterInfos), end(state.monsterInfos), isGenericMonster);
    while (nextUnknown != end(state.monsterInfos))
    {
      const auto& pos = nextUnknown->position;
      const auto hash = getTileHash<PixelARGB>(image, pos.x, pos.y);
      if (const auto detected = monsterFromPixel.find(hash); detected != end(monsterFromPixel))
        nextUnknown->type = detected->second;
      else
        complete = false;
      nextUnknown = std::find_if(nextUnknown + 1, end(state.monsterInfos), isGenericMonster);
    };
    return complete;
  }

  bool ImageProcessor::extractMonsterInfos()
  {
    auto& gameWindow = capture.getGameWindow();
    AutoRestoreMousePosition restoreMouse(gameWindow);
    bool success = true;
    for (auto& info : state.monsterInfos)
    {
      success &= extractMonsterInfoImpl(info, gameWindow, capture);
    }
    moveMouseTo(gameWindow.getDisplay(), gameWindow.getWindow(), 750, 100);
    return success;
  }

  std::vector<MonsterInfo> ImageProcessor::findMonstersInScreenshot(std::filesystem::path path)
  {
    cv::Mat_<cv::Vec3b> image = static_cast<cv::Mat_<cv::Vec3b>>(cv::imread(path.c_str(), cv::IMREAD_COLOR));
    if (image.empty())
      throw std::runtime_error("Could not read image data from " + path.string());
    if (image.size[1] != required_screen_size_x || image.size[0] != required_screen_size_y)
    {
      throw std::runtime_error("Wrong image size " + std::to_string(image.size[1]) + "x" +
                               std::to_string(image.size[0]) + " (required: " + std::to_string(required_screen_size_x) +
                               "x" + std::to_string(required_screen_size_y) + ")");
    }
    auto [infos, complete] = findMonstersImpl<PixelBGR>(image);
    return infos;
  }

} // namespace importer
