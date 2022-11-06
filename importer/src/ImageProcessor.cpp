#include "importer/ImageProcessor.hpp"

#include "importer/GameWindow.hpp"
#include "importer/ImageCapture.hpp"
#include "importer/Mouse.hpp"
#include "importer/PixelAdapters.hpp"

#include <opencv2/opencv.hpp>

#if !defined(WIN32)
#include "X11/Xlib.h"
#endif

#include <map>
#include <thread>

namespace importer
{
  namespace
  {
    constexpr int required_screen_size_x = 800;
    constexpr int required_screen_size_y = 600;

    // Identify monster levels using the red and green components of 3 specific pixels on a tile.
    static constexpr std::array<uint64_t, 10> levelFromPixels = {
        0x00FF00FF0000, 0x00FF000000FF, 0x000000FF0000, 0xFFFF0000FFFF, 0xFFFFFFFF0000,
        0xFFFFFFFFFFFF, 0x0000FF800000, 0xFF80FF80FF80, 0xFF80FF800000, 0xFF00FF000000,
    };

    struct OffsetsLinux
    {
      // Pixel positions for level identification are (3, 18), (4, 22) (3, 24) on Linux.
      static constexpr int level_pixel_x1 = 3;
      static constexpr int level_pixel_y1 = 18;
      static constexpr int level_pixel_x2 = 4;
      static constexpr int level_pixel_y2 = 22;
      static constexpr int level_pixel_x3 = 3;
      static constexpr int level_pixel_y3 = 24;

      // On Linux/X11, the window contents are shifted one pixel up: the topmost row is missing, and a black row of
      // pixels appears at the bottom of the window.
      static constexpr int window_y = -1;
    };

    struct OffsetsWindows
    {
      // The numbers are scaled and positioned a little differently on Windows.
      static constexpr int level_pixel_x1 = 4;
      static constexpr int level_pixel_y1 = 18;
      static constexpr int level_pixel_x2 = 5;
      static constexpr int level_pixel_y2 = 21;
      static constexpr int level_pixel_x3 = 4;
      static constexpr int level_pixel_y3 = 24;

      static constexpr int window_y = 0;
    };

#if !defined(WIN32)
    using DefaultOffsets = OffsetsLinux;
#else
    using DefaultOffsets = OffsetsWindows;
#endif

    template <typename PixelFunc, class Offsets>
    std::optional<Level> getMonsterLevel(const cv::Mat& dungeon, int x, int y)
    {
      // Level number (if any) is a 16x16 sprite overlayed in the lower left corner of a 30x30 tile.
      const int start_x = x * 30;
      const int start_y = y * 30 + Offsets::window_y;
      // Use a suitable combination of 3 pixels to tell different numbers apart
      const auto pixel1 =
          PixelFunc(dungeon, Column{start_x + Offsets::level_pixel_x1}, Row{start_y + Offsets::level_pixel_y1});
      const auto pixel2 =
          PixelFunc(dungeon, Column{start_x + Offsets::level_pixel_x2}, Row{start_y + Offsets::level_pixel_y2});
      const auto pixel3 =
          PixelFunc(dungeon, Column{start_x + Offsets::level_pixel_x3}, Row{start_y + Offsets::level_pixel_y3});
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

    template <typename PixelFunc, class Offsets>
    inline uint32_t getTileHash(const cv::Mat& tile, int x, int y)
    {
      return PixelFunc(tile, Column{x * 30 + 12}, Row{y * 30 + Offsets::window_y + 5}).rgb();
    }

    template <typename PixelFunc, class Offsets>
    inline bool hasHealthBar(const cv::Mat& tile, int x, int y)
    {
      const auto pixel = PixelFunc(tile, Column{x * 30 + 29}, Row{y * 30 + Offsets::window_y + 29}).rgb();
      return pixel == 0xFF0000 || pixel == 0x00FF00;
    }

    // Find all monsters on map shown in the input image.
    // Returns info about monsters and a flag that indicates whether all monsters could be identified.
    template <typename PixelFunc, class Offsets>
    std::pair<std::vector<MonsterInfo>, bool> findMonstersImpl(const cv::Mat& image)
    {
      std::pair<std::vector<MonsterInfo>, bool> result;
      auto& [monsters, complete] = result;
      complete = true;
      for (unsigned index = 0u; index < 400u; ++index)
      {
        const unsigned char x = index % 20;
        const unsigned char y = static_cast<unsigned char>(index / 20);
        const auto monsterLevel = getMonsterLevel<PixelFunc, Offsets>(image, x, y);
        if (monsterLevel)
        {
          const auto hash = getTileHash<PixelFunc, Offsets>(image, x, y);
          auto monsterType = MonsterType::Generic;
          if (auto detected = monsterFromPixel.find(hash); detected != end(monsterFromPixel))
            monsterType = detected->second;
          else
            complete = false;
          monsters.emplace_back(MonsterInfo{
              TilePosition{x, y}, monsterType, *monsterLevel, hasHealthBar<PixelFunc, Offsets>(image, x, y), {}, hash});
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
      auto countBrightPixels = [&, height = imagePart.size[0]](int x)
      {
        int count = 0;
        for (int y = 0; y < height; ++y)
        {
          if (PixelFunc(imagePart, Column{x}, Row{y}).g() >= 189)
            ++count;
        }
        return count;
      };
      auto parseNumbers = [&, x = 0, width = imagePart.size[1]]() mutable -> std::optional<int>
      {
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
      moveMouseTo(gameWindow, {30 * infoToUpdate.position.x + 15, 30 * infoToUpdate.position.y + 15});
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(100ms);
      auto image = capture.asMatrix();
      auto hitpoints = extract_hp_or_mp<PixelARGB>(image(cv::Rect(651, 430, 100, 10)));
      if (hitpoints)
      {
        infoToUpdate.health = hitpoints;
        return true;
      }
      return false;
    }

    /** Confirm that the image has the expected game window size
     *  Throws std::runtime_error with appropriate message on problem
     **/
    void verifyImageSize(const cv::Mat& imageData)
    {
      if (imageData.dims < 2)
        throw std::runtime_error("Invalid image data");
      if (imageData.size[1] != required_screen_size_x || imageData.size[0] != required_screen_size_y)
        throw std::runtime_error("Wrong image size " + std::to_string(imageData.size[1]) + "x" +
                                 std::to_string(imageData.size[0]) +
                                 " (required: " + std::to_string(required_screen_size_x) + "x" +
                                 std::to_string(required_screen_size_y) + ")");
    }

    template <class Offsets>
    std::vector<MonsterInfo> findMonstersInScreenshotImpl(std::string path)
    {
      cv::Mat_<cv::Vec3b> image = static_cast<cv::Mat_<cv::Vec3b>>(cv::imread(path, cv::IMREAD_COLOR));
      if (image.empty())
        throw std::runtime_error("Could not read image data from " + path);
      verifyImageSize(image);
      auto [infos, complete] = findMonstersImpl<PixelBGR, Offsets>(image);
      return infos;
    }
  } // namespace

  ImageProcessor::ImageProcessor(ImageCapture& capture)
    : capture(capture)
  {
  }

  bool ImageProcessor::findMonsters(int numRetries, int retryDelayInMilliseconds)
  {
    auto image = capture.asMatrix();
    verifyImageSize(image);
    auto [infos, complete] = findMonstersImpl<PixelARGB, DefaultOffsets>(image);
    state.monsterInfos = std::move(infos);
    while (!complete && numRetries-- > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds{retryDelayInMilliseconds});
      complete = retryFindMonsters();
    }
    return complete;
  }

  bool ImageProcessor::retryFindMonsters()
  {
    auto image = capture.asMatrix();
    verifyImageSize(image);
    bool complete = true;
    auto isGenericMonster = [](const auto& monster) { return monster.type == MonsterType::Generic; };
    auto nextUnknown = std::find_if(begin(state.monsterInfos), end(state.monsterInfos), isGenericMonster);
    while (nextUnknown != end(state.monsterInfos))
    {
      const auto& pos = nextUnknown->position;
      const auto hash = getTileHash<PixelARGB, DefaultOffsets>(image, pos.x, pos.y);
      if (const auto detected = monsterFromPixel.find(hash); detected != end(monsterFromPixel))
        nextUnknown->type = detected->second;
      else
        complete = false;
      nextUnknown = std::find_if(nextUnknown + 1, end(state.monsterInfos), isGenericMonster);
    };
    return complete;
  }

  bool ImageProcessor::extractMonsterInfos(bool smart)
  {
    auto& gameWindow = capture.getGameWindow();
    AutoRestoreMousePosition restoreMouse(gameWindow);
    bool success = true;
    for (auto& info : state.monsterInfos)
    {
      if (info.hasHealthBar || !smart)
        success &= extractMonsterInfoImpl(info, gameWindow, capture);
    }
    moveMouseTo(gameWindow, {750, 100});
    return success;
  }

  std::vector<MonsterInfo> ImageProcessor::findMonstersInScreenshot(std::string path)
  {
    return findMonstersInScreenshotImpl<DefaultOffsets>(std::move(path));
  }

  std::vector<MonsterInfo> ImageProcessor::findMonstersInScreenshotLinux(std::string path)
  {
    return findMonstersInScreenshotImpl<OffsetsLinux>(std::move(path));
  }

  std::vector<MonsterInfo> ImageProcessor::findMonstersInScreenshotWindows(std::string path)
  {
    return findMonstersInScreenshotImpl<OffsetsWindows>(std::move(path));
  }

} // namespace importer
