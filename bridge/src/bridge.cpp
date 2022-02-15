// Include OpenCV before any X11 headers
#include <opencv2/opencv.hpp>

#include "bridge/GameWindow.hpp"
#include "bridge/ImageCapture.hpp"
#include "bridge/Mouse.hpp"

#include "engine/Monster.hpp"
#include "engine/StrongTypes.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace
{
  // On Linux/X11, the window contents are shifted one pixel up: the topmost row is missing, and a black row of pixels
  // appears at the bottom of the window
  constexpr int y_offset = -1;

  constexpr int required_screen_size_x = 800;
  constexpr int required_screen_size_y = 600;

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
      {0xA2FBD5, MonsterType::Changeling},
      {0xA3FBD5, MonsterType::Changeling},
      {0xA3FCD5, MonsterType::Changeling},
      {0x9B7036, MonsterType::DesertTroll},
      {0x1285FD, MonsterType::Djinn},
      {0x79341C, MonsterType::Minotaur},
      {0x718A3F, MonsterType::Naga},
      {0xB6AAA1, MonsterType::Ratling},
      {0xBBD6D5, MonsterType::Vampire},
  };

  // Red and green components of 3 specific pixels on a tile (3, 18), (4, 22) (3, 24)
  static constexpr std::array<uint64_t, 10> levelFromPixels = {
      0x00FF00FF0000, 0x00FF000000FF, 0x000000FF0000, 0xFFFF0000FFFF, 0xFFFFFFFF0000,
      0xFFFFFFFFFFFF, 0x0000FF800000, 0xFF80FF80FF80, 0xFF80FF800000, 0xFF00FF000000,
  };

  [[maybe_unused]] auto getTile(const cv::Mat& dungeon, int x, int y)
  {
    static_assert(y_offset <= 0 && y_offset > -30);
    if constexpr (y_offset == 0)
    {
      return dungeon(cv::Rect(x * 30, y * 30, 30, 30));
    }
    else if (y > 0)
      return dungeon(cv::Rect(x * 30, y * 30 + y_offset, 30, 30));
    else
    {
      auto partialTile = dungeon(cv::Rect(x * 30, 0, 30, 30 + y_offset));
      cv::Mat tile = cv::Mat::zeros(cv::Size(30, 30), CV_8UC4);
      partialTile.rowRange(0, 30 + y_offset).copyTo(tile.rowRange(-y_offset, 30));
      return tile;
    }
  }

  using Column = NamedType<int, struct ColumnParameter>;
  using Row = NamedType<int, struct RowParameter>;

  struct PixelBGR
  {
    PixelBGR(const cv::Mat& image, Column col, Row row)
      : _v(image.at<cv::Vec3b>(row.get(), col.get()))
    {
    }
    constexpr uint8_t r() const { return _v.val[2]; }
    constexpr uint8_t g() const { return _v.val[1]; }
    constexpr uint8_t b() const { return _v.val[0]; }
    constexpr uint32_t rgb() const
    {
      return (static_cast<uint32_t>(_v.val[2]) << 16) + (static_cast<uint32_t>(_v.val[1]) << 8) +
             static_cast<uint32_t>(_v.val[0]);
    }
    constexpr uint32_t rg() const { return (static_cast<uint32_t>(_v.val[2]) << 8) + static_cast<uint32_t>(_v.val[1]); }

  private:
    cv::Vec3b _v;
  };

  struct PixelARGB
  {
    PixelARGB(const cv::Mat& image, Column col, Row row)
      : _v(image.at<uint32_t>(row.get(), col.get()) & 0xFFFFFF)
    {
    }
    constexpr uint8_t r() const { return (_v & 0xFF0000) >> 16; }
    constexpr uint8_t g() const { return (_v & 0xFF00) >> 8; }
    constexpr uint8_t b() const { return _v & 0xFF; }
    constexpr uint32_t rgb() const { return _v; }
    constexpr uint32_t rg() const { return _v >> 8; }

  private:
    uint32_t _v;
  };

  template <typename PixelFunc>
  std::optional<Level> getMonsterLevel(const cv::Mat& dungeon, int x, int y)
  {
    // Level number (if any) is a 16x16 sprite overlayed in the lower left corner of a 30x30 tile.
    const int start_x = x * 30;
    const int start_y = y * 30 + y_offset;
    // Several suitable combinations of 3 pixels exist that suffice to tell them apart, we use (3, 18), (4, 22) (3, 24).
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

  template <typename PixelFunc>
  inline int32_t getTileHash(const cv::Mat& tile, int x, int y)
  {
    return PixelFunc(tile, Column{x * 30 + 12}, Row{y * 30 + y_offset + 5}).rgb();
  }

  using HealthInfo = std::pair<int, int>;

  struct Position
  {
    int x;
    int y;
  };

  struct MonsterInfo
  {
    Position position;
    MonsterType type;
    Level level;
    std::optional<HealthInfo> health;
  };

  struct ImageInfo
  {
    std::vector<MonsterInfo> monsters;
  };

  [[nodiscard]] ImageInfo processImage(const cv::Mat& image)
  {
    ImageInfo result;
    auto getLevel = [type = image.type()] {
      if (type == CV_8UC4)
        return getMonsterLevel<PixelARGB>;
      else
      {
        assert(type == CV_8UC3);
        return getMonsterLevel<PixelBGR>;
      }
    }();
    auto getMonster = [type = image.type()] {
      if (type == CV_8UC4)
        return getTileHash<PixelARGB>;
      else
      {
        assert(type == CV_8UC3);
        return getTileHash<PixelBGR>;
      }
    }();
    for (unsigned index = 0u; index < 400u; ++index)
    {
      const unsigned char x = index % 20;
      const unsigned char y = static_cast<unsigned char>(index / 20);
      const auto level = getLevel(image, x, y);
      if (!level)
        continue;
      const auto monsterType = [hash = getMonster(image, x, y), level = level->get()] {
        if (auto detected = monsterFromPixel.find(hash); detected != end(monsterFromPixel))
        {
          printf("%s level %i\t", toString(detected->second), level);
          return detected->second;
        }
        else
        {
          printf("unknown monster level %i (hash 0x%06X)\t", level, hash);
          return MonsterType::Generic;
        }
      }();
      result.monsters.emplace_back(MonsterInfo{Position{x, y}, monsterType, *level, {}});
    }
    if (!result.monsters.empty())
      puts("");
    return result;
  }

  // Identify monster HP and max HP by counting number of "bright" pixels per column
  constexpr std::array patterns = {52225, 11900, 22224, 22324, 23391, 23323, 50333, 13221, 63336, 33305, 0};

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
        const auto match = std::find(begin(patterns), end(patterns), nextPattern);
        if (match == end(patterns))
          return {};
        const auto index = static_cast<int>(std::distance(begin(patterns), match));
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
    if (!points || !maxPoints)
    {
      std::cerr << "Failed to extract HP or MP from image region.";
      return {};
    }
    return {{*points, *maxPoints}};
  }

  // Extract info for monster that is tagged / hovered over from sidebar
  template <typename PixelFunc>
  void updateMonsterInfo(const cv::Mat& image, MonsterInfo& monsterInfo)
  {
    monsterInfo.health = extract_hp_or_mp<PixelFunc>(image(cv::Rect(651, 430, 100, 10)));
    if (monsterInfo.health)
    {
      const MonsterStats stats(monsterInfo.type, monsterInfo.level, DungeonMultiplier{1.299f});
      if (stats.getHitPointsMax() != HitPoints{monsterInfo.health->second})
        std::cout << monsterInfo.health->first << '/' << monsterInfo.health->second
                  << " (HP max expected: " << stats.getHitPointsMax().get() << ")";
      else
        std::cout << std::endl
                  << "AssertThat(Monster(MonsterType::" << toString(monsterInfo.type) << ", Level{"
                  << (int)monsterInfo.level.get() << "}, DungeonMultiplier{x}).getHitPoints(), Equals("
                  << monsterInfo.health->second << "u));";
    }
  }

  void scanMonsters(ImageInfo& imageInfo, GameWindow& gameWindow, ImageCapture& capture)
  {
    if (imageInfo.monsters.empty())
      return;
    auto pos = getMousePosition(gameWindow.getDisplay(), 0);
    for (auto& monsterInfo : imageInfo.monsters)
    {
      moveMouseTo(gameWindow.getDisplay(), gameWindow.getWindow(), 30 * monsterInfo.position.x + 15,
                  30 * monsterInfo.position.y + 15);
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(100ms);
      std::cout << toString(monsterInfo.type) << " level " << (int)monsterInfo.level.get() << ": ";
      if (auto ximage = capture.acquire())
      {
        auto image = cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
        updateMonsterInfo<PixelARGB>(image, monsterInfo);
      }
      std::cout << '\n';
    }
    moveMouseTo(gameWindow.getDisplay(), gameWindow.getWindow(), 750, 100);
    if (pos)
      moveMouseTo(gameWindow.getDisplay(), 0, pos->first, pos->second);
  }

  [[maybe_unused]] void importState()
  {
    /** Pseudocode:
     * Grab screen
     * Identify dungeon?
     * Process all tiles:
     *  - Identify monster
     *    - Get level
     *    - Get health (Check if health bar is present? Find numeric value from side-bar)
     *  - Identify wall, corroded wall, petrified enemy, shop, altar, empty tile
     *  - Identify plants and glyphs
     *  - Identify extras: booster, gold, blood, acid, piety
     * Process hero state?
     **/
  }
} // namespace

unsigned monitorContinuous(GameWindow& gameWindow)
{
  unsigned numFrames = 0;
  ImageCapture capture(gameWindow);
  while (auto ximage = capture.acquire())
  {
    if (ximage->width != required_screen_size_x || ximage->height != required_screen_size_y)
    {
      printf("Please resize game window to %ix%i (current size: %ix%i)\n", required_screen_size_x,
             required_screen_size_y, ximage->width, ximage->height);
      cv::waitKey(1000);
      continue;
    }
    printf("Processing image...\n");
    auto image = cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
    auto result = processImage(std::move(image));
    for (int i = 0; i < 5; ++i)
    {
      // If a monster could not be identified, this could be due to an animation (monster is slowed, or there's a piety
      // token on the same square).  Take another screenshot and retry.  Attempt up to 5 extra screenshots.
      const auto isGenericMonster = [](const auto& monster) { return monster.type == MonsterType::Generic; };
      auto nextUnknown = std::find_if(begin(result.monsters), end(result.monsters), isGenericMonster);
      if (nextUnknown == end(result.monsters))
        break;
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(110ms);
      ximage = capture.acquire();
      image = cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
      auto result2 = processImage(std::move(image));
      // Overwrite generic monsters with result from second screenshot
      do
      {
        nextUnknown = std::find_if(nextUnknown + 1, end(result.monsters), isGenericMonster);
        *nextUnknown = result2.monsters[std::distance(begin(result.monsters), nextUnknown)];
      } while (nextUnknown != end(result.monsters));
    }

    scanMonsters(result, gameWindow, capture);
    cv::waitKey(1000);
    ++numFrames;
  }
  return numFrames;
}

bool processImageFromFile(const char* filename)
{
  auto image = static_cast<cv::Mat_<cv::Vec3b>>(cv::imread(filename, cv::IMREAD_COLOR));
  if (image.empty())
  {
    std::cerr << "Could not read image data from " << filename << std::endl;
    return false;
  }
  if (image.size[0] != required_screen_size_y || image.size[1] != required_screen_size_x)
  {
    std::cerr << "Wrong image size " << image.size[1] << "x" << image.size[0]
              << " (required: " << required_screen_size_x << "x" << required_screen_size_y << ")" << std::endl;
    return false;
  }

  const auto result = processImage(image);
  std::cout << result.monsters.size() << " monster(s) found." << std::endl;
  for (const auto& monsterInfo : result.monsters)
    std::cout << toString(monsterInfo.type) << " "
              << " level " << monsterInfo.level.get() << std::endl;

  return true;
}

bool processImagesFromGameWindow()
{
  GameWindow gameWindow;
  const bool imagesProcessed = monitorContinuous(gameWindow) > 0;
  if (!imagesProcessed)
    std::cerr << "Game window not found." << std::endl;
  return imagesProcessed > 0;
}

int main(int argc, char** argv)
{
  const bool success = argc >= 2 ? processImageFromFile(argv[1]) : processImagesFromGameWindow();
  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
