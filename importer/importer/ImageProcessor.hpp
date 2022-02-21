#pragma once

#include "importer/ImageCapture.hpp"
#include "importer/ImportedState.hpp"

#include <filesystem>
#include <memory>
#include <utility>

namespace importer
{
  class ImageProcessor
  {
  public:
    ImageProcessor(ImageCapture&);

    // Find monsters on map in screen capture; optionally attempt multiple times if any monster was not identified
    const std::vector<MonsterInfo>& findMonsters(int numRetries = 0, int retryDelayInMilliseconds = 100);

    // Extract additional information for monsters from sidebar
    const std::vector<MonsterInfo>& extractMonsterInfos();

    [[nodiscard]] ImportedState get() const & { return { monsterInfos }; }
    [[nodiscard]] ImportedState get() && { return { std::move(monsterInfos) }; }

    static std::vector<MonsterInfo> findMonstersInScreenshot(std::filesystem::path screenshot);

  private:
    ImageCapture& capture;
    std::vector<MonsterInfo> monsterInfos;
  };

} // namespace importer
