// Include OpenCV before any X11 headers
#include <opencv2/opencv.hpp>

#include "bridge/GameWindow.hpp"
#include "bridge/ImageCapture.hpp"
#include "bridge/Mouse.hpp"

#include "engine/Monster.hpp"
#include "engine/PositionedVector.hpp"
#include "engine/StrongTypes.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <string_view>
#include <vector>

namespace
{
  // On Linux/X11, the window contents are shifted one pixel up: the topmost row is missing, and a black row of pixels
  // appears at the bottom of the window
  constexpr int y_offset = -1;

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
      {0x718A3F, MonsterType::Naga},
      {0xBBD6D5, MonsterType::Vampire},
  };

  // Red and green components of 3 specific pixels on a tile (3, 18), (4, 22) (3, 24)
  static constexpr std::array<uint64_t, 10> levelFromPixels = {
      0x00FF00FF0000, 0x00FF000000FF, 0x000000FF0000, 0xFFFF0000FFFF, 0xFFFFFFFF0000,
      0xFFFFFFFFFFFF, 0x0000FF800000, 0xFF80FF80FF80, 0xFF80FF800000, 0xFF00FF000000,
  };
} // namespace

auto getTile(const cv::Mat& dungeon, int x, int y)
{
  if constexpr (y_offset == 0)
  {
    return dungeon(cv::Rect(x * 30, y * 30, 30, 30));
  }
  else
  {
    static_assert(y_offset < 0 && y_offset > -30);
    if (y > 0)
      return dungeon(cv::Rect(x * 30, y * 30 + y_offset, 30, 30));
    auto partialTile = dungeon(cv::Rect(x * 30, 0, 30, 30 + y_offset));
    cv::Mat tile = cv::Mat::zeros(cv::Size(30, 30), CV_8UC4);
    partialTile.rowRange(0, 30 + y_offset).copyTo(tile.rowRange(-y_offset, 30));
    return tile;
  }
}

int32_t getTileHash(const cv::Mat& tile)
{
  return tile.at<int32_t>(4, 11) & 0xFFFFFF;
}

std::optional<Level> getMonsterLevel(const cv::Mat& dungeon, int x, int y)
{
  // Level number (if any) is a 16x16 sprite overlayed in the lower left corner of a 30x30 tile.
  const int start_y = y * 30 + y_offset + 18;
  const int start_x = x * 30 + 3;
  // Several suitable combinations of 3 pixels exist that suffice to tell them apart, we use (3, 18), (4, 22) (3, 24)
  const auto pixel1 = dungeon.at<uint32_t>(start_y, start_x) & 0xFFFFFF;
  const auto pixel2 = dungeon.at<uint32_t>(start_y + 4, start_x + 1) & 0xFFFFFF;
  const auto pixel3 = dungeon.at<uint32_t>(start_y + 6, start_x) & 0xFFFFFF;
  // Level labels have no blue component
  if ((pixel1 & 0xFF) == 0 && (pixel2 & 0xFF) == 0 && (pixel3 & 0xFF) == 0)
  {
    const uint64_t hash = (static_cast<uint64_t>(pixel1) << 24) + (static_cast<uint64_t>(pixel2) << 8) +
                          (static_cast<uint64_t>(pixel3) >> 8);
    const auto iter = std::find(begin(levelFromPixels), end(levelFromPixels), hash);
    if (iter != end(levelFromPixels))
    {
      const auto index = std::distance(begin(levelFromPixels), iter);
      return Level{index + 1};
    }
  }
  return {};
}

auto getTileCropped(const cv::Mat& dungeon, int x, int y)
{
  return dungeon(cv::Rect(x * 30 + 1, y * 30, 22, 25));
}

namespace
{
  struct ImageInfo
  {
    PositionedVector<Monster> monsters;
  };

  [[nodiscard]] ImageInfo processImage(const cv::Mat& image)
  {
    ImageInfo result;
    for (unsigned index = 0u; index < 400u; ++index)
    {
      const unsigned char x = index % 20;
      const unsigned char y = static_cast<unsigned char>(index / 20);
      auto level = getMonsterLevel(image, x, y);
      if (level)
      {
        auto tile = getTileCropped(image, x, y);
        const auto monsterType = [hash = getTileHash(tile), level = level->get()] {
          if (auto detected = monsterFromPixel.find(hash); detected != end(monsterFromPixel))
          {
            printf("%s level %i\t", toString(detected->second), level);
            return detected->second;
          }
          else
          {
            printf("unknown monster level %i, hash 0x%06X\n", level, hash);
            return MonsterType::Generic;
          }
        }();
        Monster monster{monsterType, level->get(), 100};
        result.monsters.emplace_back(make_positioned(std::move(monster), Position{x, y}));
      }
    }
    puts("");
    return result;
  }

  void scanMonsters(const ImageInfo& imageInfo, GameWindow& gameWindow)
  {
    if (imageInfo.monsters.empty())
      return;
    auto pos = getMousePosition(gameWindow.getDisplay(), 0);
    for (const auto& [monster, position] : imageInfo.monsters)
    {
      moveMouseTo(gameWindow.getDisplay(), gameWindow.getWindow(), 30 * position.x + 15, 30 * position.y + 15);
    }
    if (pos)
      moveMouseTo(gameWindow.getDisplay(), 0, pos->first, pos->second);
  }
} // namespace

unsigned monitorContinuous(GameWindow& gameWindow)
{
  unsigned numFrames = 0;
  ImageCapture capture(gameWindow);
  while (auto ximage = capture.acquire())
  {
    if (ximage->width != 800 || ximage->height != 600)
    {
      printf("Please resize game window to 800x600 (current size: %ix%i)\n", ximage->width, ximage->height);
      cv::waitKey(1000);
      continue;
    }
    printf("Processing image...\n");
    auto image = cv::Mat(ximage->height, ximage->width, CV_8UC4, ximage->data);
    auto result = processImage(image);
    scanMonsters(result, gameWindow);
    cv::waitKey(1000);
    ++numFrames;
  }
  return numFrames;
}

int main()
{
  GameWindow gameWindow;
  return monitorContinuous(gameWindow) > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
