#pragma once

#include <opencv2/opencv.hpp>

namespace importer
{
  class GameWindow;

  class ImageCapture
  {
  public:
    ImageCapture(GameWindow& gameWindow)
      : gameWindow(gameWindow)
      , lastCaptureSuccessful(false)
    {
    }

    /** @brief Capture game window content as OpenCV Matrix
     *  Throws std::runtime_error if the image cannot be acquired
     *  Note: Delivers incorrect results if screen scaling is enabled (Windows)
     **/
    cv::Mat asMatrix();

    //! Returns whether the last capture was successful
    bool success() const { return lastCaptureSuccessful; }

    GameWindow& getGameWindow() const { return gameWindow; }

  private:
    GameWindow& gameWindow;
    bool lastCaptureSuccessful;
  };
} // namespace importer
