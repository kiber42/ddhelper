// Include OpenCV before any X11 headers
#include <opencv2/opencv.hpp>

#include "engine/StrongTypes.hpp"

#include "bridge/ImageCapture.hpp"

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
  const std::map<uint32_t, std::string> monsterFromPixel = {
      // Basic Monsters
      {0xCC7F54, "Bandit"},  {0x4BB56C, "Dragon Spawn"}, {0x544130, "Goat"},   {0xE65918, "Goblin"},
      {0x7C6E57, "Golem"},   {0x373E01, "Goo Blob"},     {0x0BA837, "Gorgon"}, {0xFF6666, "Meat Man"},
      {0xBA863E, "Serpent"}, {0xC74E73, "Warlock"},      {0x090D3F, "Wraith"}, {0x378A85, "Zombie"},
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

std::optional<Level> getMonsterLevel(cv::Mat& dungeon, int x, int y)
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

int main()
{
  int numFrames = 0;
  ImageCapture capture;
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
    for (int index = 0; index < 400; ++index)
    {
      auto level = getMonsterLevel(image, index % 20, index / 20);
      if (level)
      {
        auto tile = getTileCropped(image, index % 20, index / 20);
        auto hash = getTileHash(tile);
        if (auto name = monsterFromPixel.find(hash); name != end(monsterFromPixel))
          printf("%s level %i\t", name->second.data(), level->get());
        else
          printf("unknown monster level %i, hash 0x%06X\n", level->get(), hash);
        continue;
      }
    }
    puts("");
    cv::waitKey(1000);
    ++numFrames;
  }

  return numFrames != 0;
}
