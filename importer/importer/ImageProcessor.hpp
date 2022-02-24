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

    // Take screenshot and try to identify monsters on map; optionally attempt multiple times if any monster was not identified
    // Returns true if no unidentified / generic monsters remain.
    bool findMonsters(int numRetries = 0, int retryDelayInMilliseconds = 100);

    // Take screenshot and try to update information about previously unidentied monsters.
    // This might help if a monster could not previously be identified due to an animation (monster is slowed, or
    // there's a piety token on the same square).
    // Returns true if no unidentified / generic monsters remain.
    bool retryFindMonsters();

    // Move mouse over monster tiles and extract HP information from sidebar.
    // Returns true if HP could be extracted for all monsters detected by above methods.
    // If the flag is true, successfully identified monsters at 100% health will be skipped.
    // If the dungeon difficulty multiplier is known, their health can be computed directly.
    // This will produce accurate results except for slowed monsters with >100% health,
    // as no health bar is shown for those.
    bool extractMonsterInfos(bool smart);

    [[nodiscard]] const ImportedState& get() const & { return state; }

    static std::vector<MonsterInfo> findMonstersInScreenshot(std::filesystem::path screenshot);

  private:
    ImageCapture& capture;
    ImportedState state;
  };

} // namespace importer
