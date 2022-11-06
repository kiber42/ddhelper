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

    /** Take screenshot and try to identify monsters on map; optionally attempt multiple times if any monster was not identified
     *  @Returns true if no unidentified / generic monsters remain.
     *  Throws std::runtime_error if there was a problem with screenshot acquisition.
     */
    bool findMonsters(int numRetries = 0, int retryDelayInMilliseconds = 100);

    /** @brief Take screenshot and try to update information about previously unidentied monsters.
     *  This might help if a monster could not previously be identified due to an animation (monster is slowed, or
     *  there's a piety token on the same square).
     *  @returns true if no unidentified / generic monsters remain.
     *  Throws std::runtime_error if there was a problem with screenshot acquisition.
     */
    bool retryFindMonsters();

    /** Move mouse over monster tiles and extract HP information from sidebar.
     *  @Returns true if HP could be extracted for all monsters detected by above methods.
     *  If the flag is true, successfully identified monsters at 100% health will be skipped.
     *  If the dungeon difficulty multiplier is known, their health can be computed directly.
     *  This will produce accurate results except for slowed monsters with >100% health,
     *  as no health bar is shown for those.
     */
    bool extractMonsterInfos(bool smart);

    //! Return current imported state
    [[nodiscard]] const ImportedState& get() const & { return state; }

    /** Obtain information about monsters from screenshot.  Detailed HP information is not available in this mode.
     *  Calls the appropriate implementation for the current platform.
     */
    static std::vector<MonsterInfo> findMonstersInScreenshot(std::string screenshotPath);

    /** Obtain information about monsters from screenshot.  Detailed HP information is not available in this mode.
     *  Adjusted for the Linux version of the game.
     */
    static std::vector<MonsterInfo> findMonstersInScreenshotLinux(std::string screenshotPath);

    /** Obtain information about monsters from screenshot.  Detailed HP information is not available in this mode.
     *  Adjusted for the Windows version of the game.
     */
    static std::vector<MonsterInfo> findMonstersInScreenshotWindows(std::string screenshotPath);

  private:
    ImageCapture& capture;
    ImportedState state;
  };

} // namespace importer
